#define FUSE_USE_VERSION 28
#define VIRTUAL_FILE "/tujuan.txt"
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>

static char dirpath[1024];

static char virtual_content[8192];
void generate_virtual_content()
{
    virtual_content[0] = '\0';

    strcat(virtual_content, "Tujuan Mas Amba: ");

    char fname[4096];
    char line[1024];

    for (int i = 1; i <= 7; i++) {

        snprintf(fname, sizeof(fname), "%s/%d.txt", dirpath, i);

        FILE *fp = fopen(fname, "r");

        if (!fp)
            continue;

        while (fgets(line, sizeof(line), fp)) {
            char *pos = strstr(line, "KOORD:");

            if (pos != NULL) {
                pos += strlen("KOORD:");

                while (*pos == ' ')
                    pos++;
                pos[strcspn(pos, "\n")] = 0;
                strcat(virtual_content, pos);

                break;
            }
        }

        fclose(fp);
    }
    strcat(virtual_content, "\n");
}

static  int  xmp_getattr(const char *path, struct stat *stbuf)
{
    int res;
    char fpath[4096];

    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, VIRTUAL_FILE) == 0) {
       generate_virtual_content();

       stbuf->st_mode = S_IFREG | 0444;
       stbuf->st_nlink = 1;
       stbuf->st_size = strlen(virtual_content);

       return 0;
    }

    snprintf(fpath, sizeof(fpath), "%s%s", dirpath, path);

    res = lstat(fpath, stbuf);

    if (res == -1) return -errno;

    return 0;
}



static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    char fpath[4096];

    if(strcmp(path,"/") == 0)
    {
        path=dirpath;
        sprintf(fpath,"%s",path);
    } else sprintf(fpath, "%s%s",dirpath,path);

    int res = 0;

    DIR *dp;
    struct dirent *de;
    (void) offset;
    (void) fi;

    dp = opendir(fpath);

    if (dp == NULL) return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;

        memset(&st, 0, sizeof(st));

        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        res = (filler(buf, de->d_name, &st, 0));

        if(res!=0) break;
    }

    struct stat st;

    memset(&st, 0, sizeof(st));

    st.st_mode = S_IFREG | 0444;
    st.st_nlink = 1;

    filler(buf, "tujuan.txt", &st, 0);

    closedir(dp);

    return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
    char fpath[4096];

    if (strcmp(path, VIRTUAL_FILE) == 0)
        return 0;

    sprintf(fpath, "%s%s", dirpath, path);

    int fd = open(fpath, fi->flags);

    if (fd == -1) return -errno;

    close(fd);
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    char fpath[4096];

    if (strcmp(path, VIRTUAL_FILE) == 0) {
        generate_virtual_content();
        size_t len = strlen(virtual_content);

    if (offset < len) {
       if (offset + size > len)
          size = len - offset;

       memcpy(buf, virtual_content + offset, size);
    }
    else
    {
       size = 0;
    }
    return size;
    }

    if(strcmp(path,"/") == 0)
    {
        path=dirpath;

        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);

    int res = 0;
    int fd = 0 ;

    (void) fi;

    fd = open(fpath, O_RDONLY);

    if (fd == -1) return -errno;

    res = pread(fd, buf, size, offset);

    if (res == -1) res = -errno;

    close(fd);

    return res;
}



static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .read = xmp_read,
    .open = xmp_open,
};



int  main(int  argc, char *argv[])
{
    realpath(argv[1], dirpath);
    argv[1] = argv[2];
    argc--;

    umask(0);

    return fuse_main(argc, argv, &xmp_oper, NULL);
}
