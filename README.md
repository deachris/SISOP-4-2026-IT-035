# SISOP-4-2026-IT-035

**Dikerjakan oleh: Dea Chrisna Butarbutar - 5027251035**

## Reporting

### Soal 1
**Save Asisten Kenz**

#### Penjelasan
1. Langkah pertama adalah mengambil arsip `amba_files.zip` kemudian di unzip dan file zipnya lalu dihapus.
```$ gdown "1nLXFhptDo2mnUlZsw8pTWyAVpV49W20U"```
Setelah file zip terdownload, maka file tersebut diunzip.
```$ unzip amba_files.zip```
File zip dihapus:
```$ rm -rf amba_files.zip```

2. Membuat program fuse `kenz_rescue.c`. Saat program di-mount, semua file pada amba_files muncul di mount directory.
- Fungsi untuk menggabungkan semua KOORD:
```
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
```
Pada kode di atas, pertama adalah menghapus buffer, kemudian membuka file 1.txt sampai 7.txt satu per satu.
Di setiap file, dicari baris yang mengandung kata "KOORD:" dan teks setelah kata tersebut diambil. Setelah diambil, maka semuanya digabungkan ke virtual_content.

- Fungsi getattr()
```
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
```
Kode di atas berfungsi untuk menampilkan informasi file dari yang dicari. Untuk ukuran informasi virtual_content diambil dari gabungan koordinat yang dihitung di generate_virtual_content.

- Fungsi xmp_readdir()
```
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
```
Kode di atas adalah untuk menampilkan semua file dari `amba_files` yang isinya akan dibaca semuanya. Kemudian file `tujuan.txt` ditambahkan secara manual, sehingga total ada 8 entry di `mnt/`, sedangkan di amba_files tetap 7 entry. 

- Fungsi `xmp_open()`
```
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
```
Fungsi ini adalah untuk membuka file, misalnya jika ingin membuka virtual_file, maka dia akan langsung return.

- Fungsi `xmp_read()`
```
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
```
Pada kode di atas, isi dari file akan dibaca dan ditampilkan ke user. 

- Fungsi main()
```

```
Fungsi ini menyimpan lokasi folder amba_files sebagai sumber data, kemudian argumen dirapikan agar sesuai dengan format yang dibutuhkan fuse, dan kemudian dijalankab sehingga semua operasi yang dilakukan di folder mnt/ seperti membaca dan melihat isi folder akan ditangani oleh fungsi-fungsi yang sudah dibuat sebelumnya.

### OUTPUT
<img width="902" height="185" alt="image" src="https://github.com/user-attachments/assets/148ca510-ba29-49cb-a568-0132d1fe689d" />

<img width="901" height="406" alt="image" src="https://github.com/user-attachments/assets/6ebe98bc-5b29-4032-adef-819ed5875773" />

### Soal 2
**Poke MOO**

#### Penjelasan
Pertama-tama, server didownload dari folder release 
