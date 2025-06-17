#define FUSE_USE_VERSION 26 // Menggunakan API yang lebih kompatibel

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>

/*
 * trollfs.c
 *
 * Implementasi FUSE untuk menjebak DainTontas.
 */


// --- Konten File dan Path Penting ---

// Path untuk file flag yang menandakan jebakan sudah aktif (persisten)
#define TRAP_FLAG_FILE "/var/tmp/.daintontas_trapped"

// UID untuk user DainTontas, akan diinisialisasi di main()
static uid_t daintontas_uid = -1;

// Konten file `very_spicy_info.txt`
static const char *spicy_info_for_daintontas = "Very spicy internal developer information: leaked roadmap.docx\n";
static const char *spicy_info_for_others = "DainTontas' personal secret!!.txt\n";

// Konten yang muncul setelah jebakan aktif (ASCII Art Baru yang sudah diperbaiki)
static const char *trap_content =
    "  ______   _ _    __             _ _                       _                                        _ \n"
    " |  ____| | | |  / _|           (_) |                     (_)                                      | |\n"
    " | |__ ___| | | | |_ ___  _ __   _| |_    __ _  __ _  __ _ _ _ __    _ __ _____      ____ _ _ __ __| |\n"
    " |  __/ _ \\ | | |  _/ _ \\| '__| | | __|  / _` |/ _` |/ _` | | '_ \\  | '__/ _ \\ \\ /\\ / / _` | '__/ _` |\n"
    " | | |  __/ | | | || (_) | |    | | |_  | (_| | (_| | (_| | | | | | | | |  __/\\ V  V / (_| | | | (_| |\n"
    " |_|  \\___|_|_| |_| \\___/|_|    |_|\\__|  \\__,_|\\__, |\\__,_|_|_| |_| |_|  \\___| \\_/\\_/ \\__,_|_|  \\__,_|\n"
    "                                                __/ |                                                 \n"
    "                                               |___/                                                  \n";


// --- Implementasi Fungsi FUSE ---

// Mengecek apakah jebakan sudah aktif dengan memeriksa keberadaan file flag
static int is_trap_active() {
    return access(TRAP_FLAG_FILE, F_OK) == 0;
}

// Mengaktifkan jebakan dengan membuat file flag
static void activate_trap() {
    FILE *f = fopen(TRAP_FLAG_FILE, "w");
    if (f) {
        fclose(f);
    }
}

static int troll_getattr(const char *path, struct stat *stbuf) {
    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else if (strcmp(path, "/very_spicy_info.txt") == 0) {
        stbuf->st_mode = S_IFREG | 0444; // Read-only
        stbuf->st_nlink = 1;

        struct fuse_context *context = fuse_get_context();
        uid_t current_uid = context->uid;

        if (is_trap_active() && current_uid == daintontas_uid) {
            stbuf->st_size = strlen(trap_content);
        } else if (current_uid == daintontas_uid) {
            stbuf->st_size = strlen(spicy_info_for_daintontas);
        } else {
            stbuf->st_size = strlen(spicy_info_for_others);
        }
    } else if (strcmp(path, "/upload.txt") == 0) {
        stbuf->st_mode = S_IFREG | 0666; // Read-write
        stbuf->st_nlink = 1;
        stbuf->st_size = 0; // File kosong
    } else {
        res = -ENOENT; // No such file or directory
    }
    return res;
}

static int troll_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi) {
    (void) offset; (void) fi; // Tidak digunakan

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, "very_spicy_info.txt", NULL, 0);
    filler(buf, "upload.txt", NULL, 0);

    return 0;
}

static int troll_open(const char *path, struct fuse_file_info *fi) {
    if (strcmp(path, "/very_spicy_info.txt") != 0 && strcmp(path, "/upload.txt") != 0)
        return -ENOENT;

    if ((fi->flags & O_ACCMODE) != O_RDONLY && strcmp(path, "/very_spicy_info.txt") == 0)
        return -EACCES;

    return 0;
}

static int troll_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi) {
    (void) fi; // Tidak digunakan
    size_t len;
    const char *content = NULL;

    struct fuse_context *context = fuse_get_context();
    uid_t current_uid = context->uid;

    if (strcmp(path, "/very_spicy_info.txt") == 0) {
        if (is_trap_active() && current_uid == daintontas_uid) {
            content = trap_content;
        } else if (current_uid == daintontas_uid) {
            content = spicy_info_for_daintontas;
        } else {
            content = spicy_info_for_others;
        }
    } else if (strcmp(path, "/upload.txt") == 0) {
        return 0;
    } else {
        return -ENOENT;
    }

    len = strlen(content);
    if (offset < len) {
        if (offset + size > len) {
            size = len - offset;
        }
        memcpy(buf, content + offset, size);
    } else {
        size = 0;
    }

    return size;
}

static int troll_write(const char *path, const char *buf, size_t size,
                       off_t offset, struct fuse_file_info *fi) {
    (void) buf; (void) offset; (void) fi;

    if (strcmp(path, "/upload.txt") == 0) {
        struct fuse_context *context = fuse_get_context();
        if (context->uid == daintontas_uid) {
            printf("!!! DainTontas triggered the trap !!!\n");
            activate_trap();
        }
        return size;
    }

    return -EACCES;
}

static int troll_truncate(const char *path, off_t size) {
    (void) path;
    (void) size;
    return 0;
}

static const struct fuse_operations troll_oper = {
    .getattr = troll_getattr,
    .readdir = troll_readdir,
    .open    = troll_open,
    .read    = troll_read,
    .write   = troll_write,
    .truncate = troll_truncate,
};

int main(int argc, char *argv[]) {
    struct passwd *pw = getpwnam("DainTontas");
    if (pw == NULL) {
        fprintf(stderr, "Error: User 'DainTontas' not found. Please create the user first.\n");
        return 1;
    }
    daintontas_uid = pw->pw_uid;
    printf("DainTontas UID: %d\n", daintontas_uid);
    printf("Trap flag file will be at: %s\n", TRAP_FLAG_FILE);

    return fuse_main(argc, argv, &troll_oper, NULL);
}