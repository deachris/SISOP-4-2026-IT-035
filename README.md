# SISOP-4-2026-IT-035

**Dikerjakan oleh: Dea Chrisna Butarbutar - 5027251035**

## Reporting

### Soal 1
**Save Asisten Kenz**

#### Penjelasan
1. Langkah pertama adalah mengambil arsip `amba_files.zip` kemudian di unzip dan file zipnya lalu dihapus.
   
```$ gdown "1nLXFhptDo2mnUlZsw8pTWyAVpV49W20U"```

Setelah file zip didownload, maka file tersebut diunzip.

```$ unzip amba_files.zip```

File zip dihapus:
```$ rm -rf amba_files.zip```

3. Membuat program fuse `kenz_rescue.c`. Saat program di-mount, semua file pada amba_files muncul di mount directory.
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
Fungsi ini adalah untuk membuka file, misalnya jika ingin membuka virtual_file, maka dia akan langsung return tanpa membuka file yang ada di sistem karena file ini bersifat virtual. Jika yang dibuka buka file virtual, maka fungsi membuat path dari file asli dengan menggabungkan direktori dengan path file user. Jika gagal dibuka, maka akan mengembalikan error ke FUSE. Sebaliknya, return 0 sebagai tanda bahwa file berhasil dibuka.

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
Pada kode di atas, isi dari file akan dibaca dan ditampilkan ke user. Jika file yang akan dibaca adalah file virtual, isinya akan dibuat dahulu dan panjangnya dihitung. Kemudian, data yang diminta disalin ke buffer sesuai ukuran dan offsetnya. JIka file yang diminta bukan virtual, path dari file asli akan dibuat dan dibuka lalu dibaca dengan `pread`. Setelah selesai, file ditutup dan return jumlah byte yang dibaca tadi.

- Fungsi main()
```
int  main(int  argc, char *argv[])
{
    realpath(argv[1], dirpath);
    argv[1] = argv[2];
    argc--;

    umask(0);

    return fuse_main(argc, argv, &xmp_oper, NULL);
}
```
Fungsi ini menyimpan lokasi folder amba_files sebagai sumber data, kemudian argumen dirapikan agar sesuai dengan format yang dibutuhkan fuse, dan kemudian dijalankab sehingga semua operasi yang dilakukan di folder mnt/ seperti membaca dan melihat isi folder akan ditangani oleh fungsi-fungsi yang sudah dibuat sebelumnya.

### OUTPUT
1. Download `amba_files.zip`
<img width="902" height="185" alt="image" src="https://github.com/user-attachments/assets/148ca510-ba29-49cb-a568-0132d1fe689d" />

2. Isi `amba_files`
<img width="901" height="406" alt="image" src="https://github.com/user-attachments/assets/6ebe98bc-5b29-4032-adef-819ed5875773" />

3. Cek passthrough setelah compile `kenz_rescue.c`
<img width="907" height="226" alt="image" src="https://github.com/user-attachments/assets/ab403a5d-8026-48b3-8362-ff6357046be1" />

4. Lihat `mnt`
<img width="885" height="70" alt="image" src="https://github.com/user-attachments/assets/e1a1eb11-bd8d-441e-b982-fc192b1bf415" />

5. Lihat `amba_files`
<img width="897" height="72" alt="image" src="https://github.com/user-attachments/assets/ee9d821e-ce94-4bb3-8aa7-f16412f0c3cd" />

6. Isi `tujuan.txt`
<img width="897" height="87" alt="image" src="https://github.com/user-attachments/assets/36168d17-434b-4f62-b155-280151a3faf7" />

### Soal 2
**Poke MOO**

#### Penjelasan
Pertama-tama, server didownload dari folder release yang ada di soal. Setelah didownload, pindahkan ke folder soal_2
```
$ mv ~/Downloads/server ~/SISOP-4-2026-IT-035/soal_2/
$ chmod +x ~/SISOP-4-2026-IT-035/soal_2/server // Untuk menjalankan server
```

Project ini adalah app database dengan struktur folder database dengan program di /app/db. Program servernya sendiri dapat diakses melalui TCP Connection dengan port 9000.

Download juga notes.csv.enc yang diminta dalam soal yaitu folder `release`.
```
$ mv ~/Downloads/notes.csv.enc ~/SISOP-4-2026-IT-035/soal_2/encrypted_storage/tests/
```

## Program fuse pada fuse.c
```#define BASE_DIR "/home/ubuntu/SISOP-4-2026-IT-035/soal_2/encrypted_storage"```

Path absolutnya ada di direktori penyimpanan file.

- Fungsi `fullpath`
```
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
```
Pada kode di atas, digunakan untuk mengubah path yang diminta user menjadi path yang sebenarnya di `encrypted_storage`. Jika user mengakses folder utama mount, maka program langsung mengarahkan ke folder encrypted_storage.
Jika user mengakses folder yang sudah dibuat sebelumnya, program akan mengecek apakah folder tersebut sudah ada. Jika sudah, maka path tersebut langsung digunakan tanpa tambahan `.enc` karena tidak perlu dienkripsi.
Jika user mengakses file, misalnya di dalam folder yang sudah dibuat, program akan menambahkan `.enc` di belakang nama filenya karena file tersebut adalah yang sebenarnya disimpan di disk dan terenkripsi.

- Fungsi `xmp_getattr()`
```
static int xmp_getattr(const char *path, struct stat *stbuf,
                       struct fuse_file_info *fi) {
    char fpath[1000];
    fullpath(fpath, path);

    int res = lstat(fpath, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}
```
Fungsi ini untuk mengambil isi file dari `encrypted_storage` dan lalu diteruskan ke sistem. Fungsi ini akan diapnggil setiap sistem membutuhkan info dari file.

- Fungsi `xmp_readdir()`
```
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
```
Pada kode di atas, program membaca file dari `encrypted_storage` dan menghapus adannya `.enc` sebelum ditampilkan ke terminal user.

- Fungsi `xmp_mkdir()`
Fungsi ini sendiri adalah untuk membuat folder di `encrypted_storage`.
```
static int xmp_mkdir(const char *path, mode_t mode) {
    char fpath[1000];
    fullpath(fpath, path);

    int res = mkdir(fpath, mode);

    if (res == -1)
        return -errno;

    return 0;
}
```

- Fungsi `xmp_rmdir()`
Fungsi ini adalah untuk menghapus folder di `encrypted_storage`.
```
static int xmp_rmdir(const char *path) {
    char fpath[1000];
    fullpath(fpath, path);

    int res = rmdir(fpath);

    if (res == -1)
        return -errno;

    return 0;
}
```

- Fungsi `xmp_create()`
```
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
```
Pada kode di atas, fungsinya dipanggil saat akan membuat file baru, file baru tersebut dibuat jika belum ada dengan flag `O_CREAT`. Di sini fungsi `fullpath` yang sebelumnya dipanggil.

- Fungsi `xmp_open()`
```
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
```
Fungsi di sini adalah untuk membuka file `.enc` di `encrypted_storage` dan menyimpan file descriptor di `fi->fh` untuk dapat dibaca/digunakan selanjutnya. 

- Fungsi `xmp_read()`
```
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
```
Pada kode ini, file `.enc` tadi dibuka dan dibaca dari `encrypted_storage`. Kemudian `xor_ecrypt()` dipanggil untuk mendekripsinya sehingga diubah kembali menjadi teks asli. Setelah itu, teks asli ditampilkan ke user.

- Fungsi `xmp_write()`
```
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
```
Fungsi ini adalah untuk mengcopy data dari user ke buffer sementara. Kemudian untuk mengenkripsi dipanggil `xor_encrypt`. Data yang sudah terenkripsi ditulis ke file `.enc` di `encrypted_storage`.

- Fungsi `xmp_truncate`
Pada fungsi ini, truncate sendiri didgunakan untuk memotong ukuran file `.enc` sebelum isinya ditulis ulang agar isi file lama tidak tertinggal. 
```
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
```

- Fungsi `xmp_unlink()`
Fungsi ini sendiri digunakan agar program tahu file yang mana sebenarnya harus dihapus ketika user ingin menghapus file yaitu untuk menghapus file `.enc` dari `encrypted_storage`. 
```
static int xmp_unlink(const char *path) {

    char fpath[1000];
    fullpath(fpath, path);

    int res = unlink(fpath);

    if (res == -1)
        return -errno;

    return 0;
}
```

- Fungsi `xmp_access`
Fungsi ini digunakan untuk bisa mengakses file sehingga program akan mengecek dahulu permission file di `encrypted_storage`, apakah bisa diakses oleh user atau tidak, seperti membaca, menulis, ataupun mengeksekusi file. 
```
static int xmp_access(const char *path, int mask) {

    char fpath[1000];
    fullpath(fpath, path);

    int res = access(fpath, mask);

    if (res == -1)
        return -errno;

    return 0;
}
```

- Fungsi `utiments()`
Fungsi ini adalah unutk mengubah dan memperbarui timestamp file yaitu penacatatan waktu pembuatan atau perubahan file.
```
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
```

- Fungsi `xor_crypt()`
Kode ini digunakan untuk mengenkripsi atau mendecrypt data menggunakan operasi XOR.
```
void xor_crypt(char *data, size_t len) {

    for (size_t i = 0; i < len; i++) {
        data[i] ^= 0x76;
    }
}
```

## File `client.c`
Program client sendiri adalah untuk berinteraksi dengan server database, dihubungkan ke server kemudian menyerahkannya ke `handle_connection()`
```
void handle_connection(int sock) {
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    printf("Connected to DB Server on port %d\n", PORT);
    printf("Type HELP for available commands\n");
    printf("Type EXIT to quit\n\n");

    while (1) {
        printf("db > ");
        fgets(buffer, BUFFER_SIZE, stdin);

        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "EXIT") == 0) break;
        if (strlen(buffer) == 0) continue;

        send(sock, buffer, strlen(buffer), 0);

        int valread = read(sock, response, BUFFER_SIZE);
        if (valread > 0) {
            response[valread] = '\0';
            printf("%s\n", response);
        }
    }
}
```
Pada kode di atas, pertama sekali membaca input user kemudian jika input adalah EXIT maka akan langsung keluar dan dikirim ke server. Kemudian balasannya adakan diterima dan juga ditampilkan. Ini akan berjalan terus sampai user menginput EXIT.

## Dockerfile
```
FROM ubuntu:latest

RUN apt-get update && apt-get install -y gcc pkg-config fuse3 libfuse3-dev

WORKDIR /app

COPY . .

RUN gcc fuse.c -o fuse `pkg-config fuse3 --cflags --libs`

EXPOSE 9000

CMD ["./server"]
```
`Dockerfile` ini adalah untuk
Yang digunakan adalah base image Ubuntu terbaru. Kemudian menginstal `gcc`, `pkg-config`, `fuse3`, dan `libfuse3-dev` untuk keperluan mengcompile `fuse.c`. Folder direktori yang digunakan adalah folder `/app`.
Kemudian semua file dari folder `soal_2` dicopy ke `/app` di container. Setelah itu `fuse.c` dicompile. Untuk koneksi TCP dari `client.c` menggunakan port 9000. Ketika container start, server pun dijalankan.

### OUTPUT
1. Compile `fuse.c`
<img width="902" height="67" alt="image" src="https://github.com/user-attachments/assets/ef6737e3-7737-41c3-b207-2f729fbecb21" />

2. Membuat docker image
<img width="912" height="817" alt="image" src="https://github.com/user-attachments/assets/0142ed30-ff42-4137-bd7d-769092c80fa0" />

3. Jalankan container
<img width="905" height="188" alt="image" src="https://github.com/user-attachments/assets/2f7cba87-3b8a-4aea-8878-058f9652e001" />

4. Buat file baru
<img width="902" height="47" alt="image" src="https://github.com/user-attachments/assets/de949eda-9319-4c73-b4dc-ad743dda1d88" />
<img width="905" height="101" alt="image" src="https://github.com/user-attachments/assets/b8c627a3-e4c3-40ca-9706-db9287674611" />

Tampilan tree:
<img width="905" height="473" alt="image" src="https://github.com/user-attachments/assets/4ef87f64-5fc8-423f-ae9d-2419410a5967" />

5. Compile client
<img width="912" height="52" alt="image" src="https://github.com/user-attachments/assets/a7d5c0ab-e3c6-416d-a8b6-fffdd333164a" />

Membuat database:
<img width="900" height="715" alt="image" src="https://github.com/user-attachments/assets/8f6ed0c1-c643-43d7-a170-7a59ab600741" />
<img width="902" height="275" alt="image" src="https://github.com/user-attachments/assets/4bf9e4b4-2457-47cf-ad34-eb04c6b9d574" />

