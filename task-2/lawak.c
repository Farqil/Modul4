#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

static const char *dirpath = "/home/farqil/perlawakan";
static const char *config_file = "/home/farqil/Modul4/task-2/lawak.conf";

static char *filter_words[50];
static char secret_file_basename[64];
static int access_start_hour;        
static int access_end_hour;

void load_config(){
    FILE *f = fopen(config_file, "r");
    if(!f){
        fprintf(stderr, "Gagal membuka config: %s\n", config_file);
        exit(EXIT_FAILURE);
    }
    char line[256];
    while(fgets(line, sizeof(line), f)){
        char *key = strtok(line, "=");
        char *val = strtok(NULL, "\n");
        if(!key || !val) continue;
        if(strcmp(key, "FILTER_WORDS") == 0){
            int i = 0;
            char *token = strtok(val, ",");
            while(token && i < 49){
                filter_words[i++] = strdup(token);
                token = strtok(NULL, ",");
            }
            filter_words[i] = NULL;
        } 
        else if(strcmp(key, "SECRET_FILE_BASENAME") == 0){
            strncpy(secret_file_basename, val, sizeof(secret_file_basename) - 1);
            secret_file_basename[sizeof(secret_file_basename) - 1] = '\0';
        } 
        else if(strcmp(key, "ACCESS_START") == 0){
            int hour, min;
            if(sscanf(val, "%d:%d", &hour, &min) == 2) access_start_hour = hour;
        } 
        else if(strcmp(key, "ACCESS_END") == 0){
            int hour, min;
            if(sscanf(val, "%d:%d", &hour, &min) == 2) access_end_hour = hour;
        }
    }
    fclose(f);
}

void rm_ext(const char *src, char *dest){
    strcpy(dest, src);
    char *dot = strrchr(dest, '.');
    if(dot) *dot = '\0';
}

bool is_secret(const char *name){
    char tmp[256]; rm_ext(name, tmp);
    return strcmp(tmp, secret_file_basename) == 0;
}

bool is_allowed_time(){
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    return tm->tm_hour >= access_start_hour && tm->tm_hour < access_end_hour;
}

bool is_text_file(const char *path){
    const char *ext = strrchr(path, '.');
    return ext && (strcmp(ext, ".txt") == 0 || strcmp(ext, ".md") == 0 || strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0 || strcmp(ext, ".log") == 0);
}

void logging(const char *action, const char *path){
    if(strcmp(path, "/") == 0) return;
    FILE *f = fopen("/var/log/lawakfs.log", "a");
    if(!f) return;
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    uid_t uid = fuse_get_context()->uid;
    fprintf(f, "[%04d-%02d-%02d %02d:%02d:%02d] [%d] [%s] %s\n",
        tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec,
        uid, action, path);
    fclose(f);
}

static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char *to_base64(const unsigned char *buf, int len, int *out_len){
    int olen = ((len + 2) / 3) * 4;
    char *out = malloc(olen + 1);
    char *p = out;
    for(int i = 0; i < len; i += 3){
        int v = buf[i] << 16 | (i+1 < len ? buf[i+1] << 8 : 0) | (i+2 < len ? buf[i+2] : 0);
        *p++ = b64[(v >> 18) & 0x3F];
        *p++ = b64[(v >> 12) & 0x3F];
        *p++ = (i+1 < len) ? b64[(v >> 6) & 0x3F] : '=';
        *p++ = (i+2 < len) ? b64[v & 0x3F] : '=';
    }
    *p = 0;
    *out_len = p - out;
    return out;
}

void filtering(char *buf){
    char temp[8192] ={0};
    int i = 0;
    while(buf[i]){
        char word[256] ={0};
        int j = 0;
        while(buf[i] && buf[i] != ' ' && buf[i] != '\n') word[j++] = buf[i++];
        word[j] = '\0';
        bool found = false;
        for(int k = 0; filter_words[k]; k++){
            if(strcasecmp(word, filter_words[k]) == 0){
                strcat(temp, "lawak");
                found = true;
                break;
            }
        }
        if(!found && strlen(word) > 0) strcat(temp, word);
        if(buf[i] == ' ' || buf[i] == '\n') strncat(temp, &buf[i++], 1);
    }
    strcpy(buf, temp);
}

int akses(const char *path, char *out_path, bool check_time){
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", dirpath, path);
    char *path_dup = strdup(full_path);
    if(!path_dup) return -ENOMEM;
    char *dir_path = dirname(path_dup);
    const char *filename = strrchr(path, '/');
    filename = filename ? filename + 1 : path;
    DIR *dp = opendir(dir_path);
    if(!dp){
        free(path_dup);
        return -ENOENT;
    }
    struct dirent *de;
    char tmp[256];
    while((de = readdir(dp))){
        rm_ext(de->d_name, tmp);
        if(strcmp(tmp, filename) == 0){
            if(is_secret(de->d_name) && check_time && !is_allowed_time()){
                closedir(dp);
                free(path_dup);
                return -ENOENT;
            }
            snprintf(out_path, 1024, "%s/%s", dir_path, de->d_name);
            closedir(dp);
            free(path_dup);
            return 0;
        }
    }
    closedir(dp);
    free(path_dup);
    return -ENOENT;
}

static int xmp_getattr(const char *path, struct stat *stbuf){
    char p[1024];
    if(strcmp(path, "/") == 0) strcpy(p, dirpath);
    else if(akses(path, p, false) != 0) return -ENOENT;
    if(lstat(p, stbuf) == -1) return -errno;
    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t off, struct fuse_file_info *fi){
    char p[1024];
    DIR *dp; struct dirent *de;
    if(strcmp(path, "/") == 0) dp = opendir(dirpath);
    else sprintf(p, "%s%s", dirpath, path); dp = opendir(p);
    if(!dp) return -errno;
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    while((de = readdir(dp))){
        if(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
        char name[256]; rm_ext(de->d_name, name);
        filler(buf, name, NULL, 0);
    }
    closedir(dp);
    return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi){
    char p[1024];
    if(akses(path, p, true) != 0) return -ENOENT;
    int fd = open(p, O_RDONLY);
    if(fd < 0) return -errno;
    close(fd);
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    char p[1024];
    if(akses(path, p, true) != 0) return -ENOENT;
    logging("READ", path);
    int fd = open(p, O_RDONLY);
    if(fd < 0) return -errno;
    struct stat st;
    fstat(fd, &st);
    unsigned char *data = malloc(st.st_size);
    pread(fd, data, st.st_size, 0);
    close(fd);
    if(is_text_file(p)){
        memcpy(buf, data, size);
        buf[size] = '\0';
        filtering(buf);
        free(data);
        return strlen(buf);
    } 
    else{
        int olen;
        char *b64 = to_base64(data, st.st_size, &olen);
        size_t n = (offset < olen) ? ((offset + size > olen) ? olen - offset : size) : 0;
        memcpy(buf, b64 + offset, n);
        free(b64);
        free(data);
        return n;
    }
}

static int xmp_access(const char *path, int mask){
    char p[1024];
    if(strcmp(path, "/") == 0) strcpy(p, dirpath);
    else if(akses(path, p, true) != 0) return -ENOENT;
    if(access(p, mask) == -1) return -errno;
    logging("ACCESS", path);
    return 0;
}

static int tolak(){ 
  return -EROFS;
}

static struct fuse_operations xmp_oper ={
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .open    = xmp_open,
    .read    = xmp_read,
    .access  = xmp_access,
    .write   = tolak,
    .truncate= tolak,
    .create  = tolak,
    .unlink  = tolak,
    .mkdir   = tolak,
    .rmdir   = tolak,
    .rename  = tolak,
};

int main(int argc, char *argv[]){
    umask(0);
    load_config();
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
