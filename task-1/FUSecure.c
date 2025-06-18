#define FUSE_USE_VERSION 28

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

static const char *dirpath = "/home/farqil/shared_files";

int bolehakses(const char *path){
    uid_t uid = fuse_get_context()->uid;
    if(strstr(path, "/private_yuadi") == path){
        if(uid != 1001) return 0;
    }
    if(strstr(path, "/private_irwandi") == path){
        if(uid != 1002) return 0;
    }
    return 1;
}

static int xmp_getattr(const char *path, struct stat *stbuf){
    if(!bolehakses(path)) return -EACCES;
    int res;
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    res = lstat(fpath, stbuf);
    if(res == -1) return -errno;
    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
    char fpath[1000];
    if(strcmp(path, "/") == 0){
        sprintf(fpath, "%s", dirpath);
    } 
    else{
        sprintf(fpath, "%s%s", dirpath, path);
    }
    DIR *dp;
    struct dirent *de;
    (void) offset;
    (void) fi;
    dp = opendir(fpath);
    if(dp == NULL) return -errno;
    while((de = readdir(dp)) != NULL){
        char relpath[1000];
        sprintf(relpath, "%s/%s", path, de->d_name);
        if(!bolehakses(relpath)) continue;
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if(filler(buf, de->d_name, &st, 0)) break;
    }
    closedir(dp);
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    if(!bolehakses(path)) return -EACCES;
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    int fd = open(fpath, O_RDONLY);
    if(fd == -1) return -errno;
    int res = pread(fd, buf, size, offset);
    if(res == -1) res = -errno;
    close(fd);
    return res;
}

static int xmp_open(const char *path, struct fuse_file_info *fi){
    if(!bolehakses(path)) return -EACCES;
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    int fd = open(fpath, O_RDONLY);
    if(fd == -1) return -errno;
    close(fd);
    return 0;
}

static int deny(){
    return -EACCES;
}

static struct fuse_operations xmp_oper ={
    .getattr    = xmp_getattr,
    .readdir    = xmp_readdir,
    .read       = xmp_read,
    .open       = xmp_open,
    .write      = deny,
    .mkdir      = deny,
    .rmdir      = deny,
    .unlink     = deny,
    .rename     = deny,
    .create     = deny,
    .truncate   = deny,
    .chmod      = deny,
    .chown      = deny,
    .utimens    = deny,
};

int main(int argc, char *argv[]){
    umask(0);
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
