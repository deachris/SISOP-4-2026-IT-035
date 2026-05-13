#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

#define BASE_DIR "/home/ubuntu/SISOP-4-2026-IT-035/soal_2/encrypted_storage"

void xor_crypt(char *data, size_t len);

static void fullpath(char fpath[1000], const char *path) {
    if (strcmp(path, "/") == 0) {
        sprintf(fpath, "%s", BASE_DIR);
        return;
    }

    char temp[1000];
    sprintf(temp, "%s%s", BASE_DIR, path);

    struct stat st;
    if (stat(temp, &st) == 0 && S_ISDIR(st.st_mode)) {
        sprintf(fpath, "%s%s", BASE_DIR, path);
    } else {
        sprintf(fpath, "%s%s.enc", BASE_DIR, path);
    }
}

static int xmp_getattr(const char *path, struct stat *stbuf,
                       struct fuse_file_info *fi) {
    char fpath[1000];
    fullpath(fpath, path);

    int res = lstat(fpath, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_readdir(const char *path, void *buf,
                       fuse_fill_dir_t filler,
                       off_t offset,
                       struct fuse_file_info *fi,
                       enum fuse_readdir_flags flags) {

    DIR *dp;
    struct dirent *de;

    char fpath[1000];
    fullpath(fpath, path);

    dp = opendir(fpath);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL) {

        char name[256];
        strcpy(name, de->d_name);

        if (strstr(name, ".enc")) {
            name[strlen(name) - 4] = '\0';
        }

        filler(buf, name, NULL, 0, 0);
    }

    closedir(dp);
    return 0;
}

static int xmp_mkdir(const char *path, mode_t mode) {
    char fpath[1000];
    fullpath(fpath, path);

    int res = mkdir(fpath, mode);

    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_rmdir(const char *path) {
    char fpath[1000];
    fullpath(fpath, path);

    int res = rmdir(fpath);

    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_create(const char *path,
                      mode_t mode,
                      struct fuse_file_info *fi) {

    char fpath[1000];
    fullpath(fpath, path);

    int fd = open(fpath, fi->flags | O_CREAT, mode);

    if (fd == -1)
        return -errno;

    close(fd);
    return 0;
}

static int xmp_open(const char *path,
                    struct fuse_file_info *fi) {

    char fpath[1000];
    fullpath(fpath, path);

    int fd = open(fpath, fi->flags);

    if (fd == -1)
        return -errno;

    fi->fh = fd;

    return 0;
}

static int xmp_read(const char *path,
                    char *buf,
                    size_t size,
                    off_t offset,
                    struct fuse_file_info *fi) {

    char fpath[1000];
    fullpath(fpath, path);

    int fd = open(fpath, O_RDONLY);

    if (fd == -1)
        return -errno;

    int res = pread(fd, buf, size, offset);

    if (res == -1)
        res = -errno;
    else
        xor_crypt(buf, res);

    close(fd);

    return res;
}

static int xmp_write(const char *path,
                     const char *buf,
                     size_t size,
                     off_t offset,
                     struct fuse_file_info *fi) {

    char fpath[1000];
    fullpath(fpath, path);

    char *enc = malloc(size);

    memcpy(enc, buf, size);

    xor_crypt(enc, size);

    int fd = open(fpath, O_WRONLY);

    if (fd == -1) {
        free(enc);
        return -errno;
    }

    int res = pwrite(fd, enc, size, offset);

    if (res == -1)
        res = -errno;

    free(enc);
    close(fd);

    return res;
}

static int xmp_truncate(const char *path,
                        off_t size,
                        struct fuse_file_info *fi) {

    char fpath[1000];
    fullpath(fpath, path);

    int res = truncate(fpath, size);

    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_unlink(const char *path) {

    char fpath[1000];
    fullpath(fpath, path);

    int res = unlink(fpath);

    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_access(const char *path, int mask) {

    char fpath[1000];
    fullpath(fpath, path);

    int res = access(fpath, mask);

    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_utimens(const char *path,
                       const struct timespec tv[2],
                       struct fuse_file_info *fi) {

    char fpath[1000];
    fullpath(fpath, path);

    int res = utimensat(0, fpath, tv, 0);

    if (res == -1)
        return -errno;

    return 0;
}

void xor_crypt(char *data, size_t len) {

    for (size_t i = 0; i < len; i++) {
        data[i] ^= 0x76;
    }
}

static struct fuse_operations xmp_oper = {
    .getattr  = xmp_getattr,
    .readdir  = xmp_readdir,
    .mkdir    = xmp_mkdir,
    .rmdir    = xmp_rmdir,
    .create   = xmp_create,
    .open     = xmp_open,
    .read     = xmp_read,
    .write    = xmp_write,
    .truncate = xmp_truncate,
    .unlink   = xmp_unlink,
    .access   = xmp_access,
    .utimens  = xmp_utimens,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
