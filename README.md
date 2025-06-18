|    NRP     |      Name      |
| :--------: | :------------: |
| 5025241015 | Farrel Aqilla Novianto |
| 5025221102 | Marco Marcello Hugo |

# Praktikum Modul 4 _(Module 4 Lab Work)_

</div>

### Daftar Soal _(Task List)_

- [Task 1 - FUSecure](/task-1/)

- [Task 2 - LawakFS++](/task-2/)

- [Task 3 - Drama Troll](/task-3/)

- [Task 4 - LilHabOS](/task-4/)

### Laporan Resmi Praktikum Modul 4 _(Module 4 Lab Work Report)_

## Task 1 (Farrel)

## a. Setup Direktori dan Pembuatan User
## b. Akses Mount Point
## c. Read-Only untuk Semua User
## d. Akses Public Folder
## e. Akses Private Folder yang Terbatas

## Task 2 (Farrel)

Sebelum melanjutkan saya membuat source directory `perlawakan` di `home/farqil` serta membuat FUSE mount point di `home/farqil/Modul4/task-2/mnt/lawak`. setelah membuat file `lawak.c` maka saya compile dengan sintaks

````gcc -Wall `pkg-config fuse --cflags` lawak.c -o output `pkg-config fuse --libs```` 

setelah itu saya mount ke mount point dengan sintaks `./output mnt/lawak` untuk poin a), b), dan c) yang nanti berganti menjadi `sudo ./output mnt/lawak/ -o allow_other` untuk poin d) dan e) karena dibutuhkan root untuk menulis log ke /var/log lalu penggunaan `-o allow_other` agar semua user bisa mengakses FUSE mount point.

## a. Ekstensi File Tersembunyi
Semua file yang ditampilkan dalam FUSE mountpoint harus memiliki ekstensi yang disembunyikan. Misalnya, file lawal.txt hanya akan muncul sebagai lawak dalam hasil perintah ls. Namun, akses terhadap file (misalnya dengan cat) tetap harus memetakan ke file asli (lawak.txt). 

Pada fitur ini, fungsi utama yang dimodifikasi adalah readdir. Dalam fungsi ini, setiap entri yang ditampilkan kepada pengguna akan dipotong bagian ekstensinya (yaitu string setelah titik terakhir .). Hal ini dilakukan agar pengguna hanya melihat nama dasar dari file. Saat file diakses, misalnya cat /mnt/lawak/document, maka pada fungsi-fungsi seperti access, getattr, dan open, dilakukan pencocokan nama dasar tersebut dengan nama file yang ada di direktori sumber.

```c
void rm_ext(const char *src, char *dest) {
    strcpy(dest, src);
    char *dot = strrchr(dest, '.');
    if (dot) *dot = '\0';
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t off, struct fuse_file_info *fi) {
    char p[1024];
    DIR *dp; struct dirent *de;
    if (strcmp(path, "/") == 0) dp = opendir(dirpath);
    else { sprintf(p, "%s%s", dirpath, path); dp = opendir(p); }
    if (!dp) return -errno;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    while ((de = readdir(dp))) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
        char name[256]; rm_ext(de->d_name, name);
        filler(buf, name, NULL, 0);
    }
    closedir(dp);
    return 0;
}
```
Direktori asli:

![Screenshot from 2025-06-18 05-21-08](https://github.com/user-attachments/assets/6fce5497-787a-452b-96fd-5a143b6cd11b)

Direktori FUSE:

![Screenshot from 2025-06-18 05-21-42](https://github.com/user-attachments/assets/3fa7b7a3-2c0a-430a-8a95-a056fa6b2403)

## b. Akses Berbasis Waktu untuk File Secret
File yang nama dasarnya mengandung "secret" hanya bisa diakses pada pukul 08:00 hingga 18:00. Di luar jam tersebut, file harus dianggap tidak ada (ditolak dengan ENOENT). Logika pembatasan waktu diterapkan di beberapa fungsi, seperti access, getattr, dan readdir. File dicek apakah namanya mengandung kata secret, dan jika iya, sistem akan mengecek waktu saat ini. Jika tidak dalam rentang waktu yang diizinkan, maka file tidak akan ditampilkan atau diakses.

```c
bool is_secret(const char *name) {
    char tmp[256]; rm_ext(name, tmp);
    return strcmp(tmp, secret_file_basename) == 0;
}

bool is_allowed_time() {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    return tm->tm_hour >= access_start_hour && tm->tm_hour < access_end_hour;
}
```
![Screenshot from 2025-06-18 05-23-13](https://github.com/user-attachments/assets/398012a0-6a3e-4239-8a6e-66b92cab38db)

`Note` ENOENT karena saya akses pada jam 05.00 yang dimana diluar batas akses

## c. Filtering Konten Dinamis
Semua file teks yang dibaca akan disensor: kata-kata tertentu seperti ducati, mu, dll akan diganti menjadi lawak. Untuk file biner, isi file akan ditampilkan dalam bentuk base64 secara dinamis. Jenis file ditentukan berdasarkan hasil eksekusi file --mime-type. Jika hasilnya text/plain, maka dilakukan filtering dengan mengganti kata-kata dari daftar ke kata "lawak". Sedangkan jika tipe file adalah binary (non-teks), maka kontennya dikonversi ke base64 saat dibaca.

```c
static const char *kata_lawak[] = {"ducati","ferrari","mu","chelsea","prx","onic","sisop", NULL};

bool is_text_file(const char *path) {
    const char *ext = strrchr(path, '.');
    return ext && (strcmp(ext, ".txt")==0 || strcmp(ext, ".md")==0 || strcmp(ext, ".c")==0 || strcmp(ext, ".h")==0 || strcmp(ext, ".log")==0);
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

void filter_lawak(char *buf){
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
```
`cat` di direktori asli:

![Screenshot from 2025-06-18 05-25-58](https://github.com/user-attachments/assets/178a146b-42d1-4677-9563-6b7a2e94cab8)

`cat` di direktori FUSE:

![Screenshot from 2025-06-18 05-24-29](https://github.com/user-attachments/assets/7994ade1-9c76-4412-8e0b-c8d44ecf61ba)
![Screenshot from 2025-06-18 05-25-26](https://github.com/user-attachments/assets/2ba30d83-a0f9-445b-aa31-7f9c5810cebc)

## d. Logging Akses
Setiap file yang berhasil diakses (READ, ACCESS) harus dicatat ke file /var/log/lawakfs.log. File log akan menuliskan timestamp, UID, jenis akses, dan path file. Logging hanya dilakukan jika akses berhasil.

```c
void tulis_log(const char *action, const char *path){
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
```
Lalu saya menjalankan `tail /var/log/lawakfs.log` untuk melihat log terbaru

![Screenshot from 2025-06-18 05-28-45](https://github.com/user-attachments/assets/7babedee-6202-4525-8d48-e0cb0c87b876)

## d. Konfigurasi
Semua parameter konfigurasi, seperti nama file secret, daftar kata filter, dan rentang waktu akses, harus disimpan dalam file lawak.conf dan bisa diubah sewaktu-waktu tanpa recompile. Dimana saya isi file `lawak.conf` dengan:
```conf
FILTER_WORDS=ducati,ferrari,mu,chelsea,prx,onic,sisop
SECRET_FILE_BASENAME=secret
ACCESS_START=08:00
ACCESS_END=18:00
```
File ini dibaca sekali di awal program menggunakan fungsi load_config(). Isi konfigurasi digunakan secara global di seluruh modul FUSE.
```c
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

int main(int argc, char *argv[]){
    umask(0);
    load_config();
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
```
Setelah saya compile dan mount kembali, command-command yang sudah saya buat sebelumnya masih berfungsi dan sesuai

![Screenshot from 2025-06-18 05-34-58](https://github.com/user-attachments/assets/2b0255e2-8ab0-4ea5-95d4-8f05a0368404)

## Task 3 (Marco)
## a. Persiapan: Pembuatan Akun Pengguna (Poin a)
Langkah pertama adalah membuat tiga akun pengguna lokal (DainTontas, SunnyBolt, Ryeku) yang akan menjadi aktor dalam skenario ini.

### 1. Buat user:
Perintah useradd dengan opsi -m digunakan untuk membuat pengguna baru sekaligus direktori home untuk mereka.

```bash 
sudo useradd -m DainTontas
sudo useradd -m SunnyBolt
sudo useradd -m Ryeku
```

![Screenshot From 2025-06-17 00-26-24](https://github.com/user-attachments/assets/d7d8e28b-1211-4292-bb1c-8f59b8331bba)

### 2. Atur password:
Selanjutnya, passwd digunakan untuk mengatur kata sandi agar setiap akun dapat digunakan untuk login.

``` bash
sudo passwd DainTontas
sudo passwd SunnyBolt
sudo passwd Ryeku
```

![Screenshot From 2025-06-17 00-27-56](https://github.com/user-attachments/assets/9d733b2a-206e-4bbf-aaeb-07ff0e563db5)

## b. Jebakan Troll
Langkah ini mencakup pembuatan titik kait (mount point), kompilasi kode C, dan pemasangan FUSE filesystem.

```
# Membuat direktori untuk mount point
sudo mkdir -p /mnt/troll

# Mengompilasi kode sumber FUSE
gcc -Wall troll.c -o troll -D_FILE_OFFSET_BITS=64 -lfuse

# Menjalankan FUSE filesystem di background
sudo ./troll /mnt/troll -o allow_other
```

Setelah filesystem berhasil dipasang, isinya diverifikasi untuk memastikan kedua file jebakan (very_spicy_info.txt dan upload.txt) telah ada.

```
# Memverifikasi isi filesystem
ls -l /mnt/troll
```

![image](https://github.com/user-attachments/assets/c08f3303-05bc-4426-8ad6-47a9e31e7dda)

## c. Jebakan Troll (Berlanjut)
Konten very_spicy_info.txt diperiksa dari sudut pandang pengguna yang berbeda untuk memastikan logika kondisional bekerja.

1. Jika dibuka oleh SunnyBolt:
Saat file dibuka oleh pengguna selain DainTontas, konten rahasia yang asli akan ditampilkan.

```
sudo -u SunnyBolt cat /mnt/troll/very_spicy_info.txt
```

output :

![image](https://github.com/user-attachments/assets/639e0428-45af-4882-8650-60174358200c)

2. Jika dibuka oleh DainTontas (sebelum jebakan aktif):
Saat file dibuka oleh DainTontas, konten yang ditampilkan adalah umpan untuk menipunya.

```
sudo -u DainTontas cat /mnt/troll/very_spicy_info.txt
```

![image](https://github.com/user-attachments/assets/102e7786-3eb4-4d92-a60f-69889c0101bf)

## d. trap
Jebakan diaktifkan ketika DainTontas menulis ke file upload.txt. Perilaku ini bersifat persisten berkat pembuatan file flag di /var/tmp/.

1. DainTontas memicu jebakan:

```
sudo -u DainTontas sh -c 'echo "upload" > /mnt/troll/upload.txt'
```

Tindakan ini akan membuat file flag /var/tmp/.daintontas_trapped untuk memastikan jebakan bersifat persisten.

2. Verifikasi setelah jebakan aktif:
Sekarang, jika DainTontas mencoba membaca file itu lagi, hasilnya akan berbeda.

```
sudo -u DainTontas cat /mnt/troll/very_spicy_info.txt
```
![image](https://github.com/user-attachments/assets/b228a9f3-1208-4fd6-aa53-91a1290860d3)

## Task 4 (Marco)
## 1. Implementasi dan Detail Teknis
Fungsionalitas sistem terbagi menjadi beberapa komponen utama yang saling berinteraksi.

### 1.1. Pustaka Standar (`src/std_lib.c`)

Karena keterbatasan compiler `bcc` yang hanya mendukung standar ANSI C (C89), diperlukan implementasi manual untuk fungsi-fungsi dasar.
- **Fungsi Aritmatika**: `div(int a, int b)` dan `mod(int a, int b)` dibuat untuk melakukan operasi pembagian dan modulo.
- **Manipulasi String**: Fungsi krusial seperti `strlen`, `strcpy`, `strcmp`, dan `strcat` diimplementasikan dari dasar. Fungsi `strcmp` dirancang untuk mengembalikan nilai *boolean-like* (1 untuk sama, 0 untuk tidak sama) agar mudah digunakan dalam struktur kondisional `if`.
- **Manajemen Memori**: `clear(byte* buf, unsigned int size)` disediakan untuk membersihkan buffer sebelum digunakan.

### 1.2. Titik Masuk dan Fungsi Assembly (`src/kernel.asm`)

File ini menjadi fondasi eksekusi kernel dan jembatan antara C dan instruksi level rendah.
- **`GLOBAL` Directives**: Simbol `_interrupt` dan `_putInMemory` diekspor menggunakan direktif `GLOBAL` agar dapat diakses oleh *linker* dan dipanggil dari file C (`kernel.c`).
- **Entry Point (`start:`)**: Dibuat sebuah *startup stub* di awal file. Bootloader akan melompat ke alamat ini. Tugas utama stub ini adalah melakukan `call _main` untuk mentransfer kontrol eksekusi ke fungsi `main()` di `kernel.c`. Ini menyelesaikan masalah kritikal di mana CPU menjalankan data acak karena `main()` tidak berada di alamat paling awal.
- **Fungsi Interrupt**: Fungsi `_interrupt` menjadi *wrapper* untuk memanggil instruksi `int` dari BIOS, memungkinkan kode C untuk mengakses layanan-layanan hardware seperti I/O layar dan keyboard.

### 1.3. Kernel Utama (`src/kernel.c`)

Ini adalah file dengan logika paling kompleks yang mengatur seluruh alur kerja shell.

#### 1.3.1. Main Loop dan Parsing Pipa
Fungsi `main()` berjalan dalam `while(1)` loop. Alurnya adalah sebagai berikut:
1.  **Baca Input**: Membaca input string dari pengguna menggunakan `readString()`.
2.  **Parsing Pipa**: Jika input tidak kosong, string akan dipindai untuk mencari karakter `|`. Setiap segmen di antara karakter pipa dianggap sebagai perintah terpisah dan pointer ke awal setiap segmen disimpan dalam array `char* commands[]`.
    ```c
    // src/kernel.c - Parsing Pipa
    num_commands = 0;
    current_pos = buf;
    commands[num_commands++] = current_pos;
    for (i = 0; i < num_commands; i++) { // <-- Perbaikan dari bug sebelumnya
        if (buf[i] == '|') {
            buf[i] = '\0';
            current_pos = &buf[i + 1];
            commands[num_commands++] = current_pos;
        }
    }
    ```

#### 1.3.2. Eksekusi Perintah Berantai (Piping)
Setelah di-parsing, perintah dieksekusi satu per satu dalam sebuah `for` loop. Mekanisme pipa diimplementasikan menggunakan dua buffer: `pipe_buffer` dan `temp_buffer`.
1.  **Iterasi 1**: `execute_command()` dijalankan untuk perintah pertama dengan `pipe_buffer` (kosong) sebagai input dan `temp_buffer` sebagai output.
2.  **Transfer Data**: Setelah perintah pertama selesai, hasilnya yang ada di `temp_buffer` disalin ke `pipe_buffer`.
3.  **Iterasi Selanjutnya**: `execute_command()` dijalankan untuk perintah kedua, kali ini dengan `pipe_buffer` (yang sudah berisi hasil dari perintah pertama) sebagai input.
4.  Proses ini berlanjut hingga semua perintah dalam rantai pipa selesai dieksekusi.

    ```c
    // src/kernel.c - Logika Eksekusi Pipa
    clear(pipe_buffer, 512);
    for (i = 0; i < num_commands; i++) {
        clear(temp_buffer, 512);
        execute_command(commands[i], pipe_buffer, temp_buffer);
        // Baris ini krusial dan merupakan perbaikan dari bug sebelumnya
        strcpy(temp_buffer, pipe_buffer); // Salin dari sumber (temp) ke tujuan (pipe)
    }
    ```

#### 1.3.3. Implementasi Perintah di `execute_command()`
Fungsi ini mem-parsing satu string perintah menjadi nama perintah dan argumennya. `trim()` digunakan untuk membersihkan spasi berlebih. Kemudian, struktur `if-else if` mengeksekusi logika yang sesuai:
- **`echo`**: Menyalin string argumen langsung ke buffer output menggunakan `strcpy(args, output)`.
- **`grep`**: Menggunakan `strstr()` untuk mencari argumen di dalam string input. Jika ditemukan, seluruh string input disalin ke buffer output.
- **`wc`**: Mengabaikan argumen, memproses string input untuk menghitung baris, kata, dan karakter. Hasilnya diformat menjadi string (misal: "1 2 11") dan disalin ke buffer output menggunakan `itoa()` dan `strcat()`.

---

## 2. Tantangan dan Proses Debugging

Proses pengembangan menghadapi beberapa tantangan signifikan yang solusinya menjadi bagian penting dari proyek ini.
1.  **Lingkungan Build Arch Linux**: `dev86` dari AUR gagal dikompilasi.
    - **Solusi**: Menggunakan **Docker** dengan image `debian:stable-slim` untuk menciptakan lingkungan build yang terisolasi dan stabil.
2.  **Kompatibilitas Compiler `bcc`**: Ditemukan banyak error kompilasi C.
    - **Solusi**: Melakukan refactoring kode agar sesuai standar ANSI C, seperti menghapus `const`, memastikan deklarasi variabel di awal blok, dan memperbaiki jumlah argumen fungsi.
3.  **Masalah Tampilan "Teks Sampah"**: Setelah boot, layar menampilkan karakter acak.
    - **Solusi**: Mengidentifikasi masalah *entry point*. Dibuat *startup stub* di `kernel.asm` untuk memanggil `_main` dan mengubah urutan *linking* di `makefile`.
4.  **Masalah `strcpy` pada Pipa**: Perintah `echo | wc` tidak menghasilkan output.
    - **Solusi**: Ditemukan bahwa argumen pada pemanggilan `strcpy` di *main loop* terbalik, menyebabkan hasil kalkulasi hilang. Memperbaiki urutan argumen `strcpy(temp_buffer, pipe_buffer)` menyelesaikan masalah ini.
5.  **Konfigurasi Bochs**: Emulator mengalami "PANIC" karena tidak menemukan ROM VGA.
    - **Solusi**: Menggunakan `sudo find /usr -name "*VGA*"` untuk menemukan path ROM yang benar dan memperbarui file `bochsrc.txt`.

---

## 3. Cara Menjalankan Proyek

Cara paling andal untuk menjalankan proyek ini adalah dengan menggunakan skrip otomatis yang telah disiapkan.

**Prasyarat:** Docker dan `make`.

1.  Berikan izin eksekusi pada skrip:
    ```bash
    chmod +x build_and_run.sh
    ```
2.  Jalankan skrip tersebut:
    ```bash
    ./build_and_run.sh
    ```
3.  Skrip akan secara otomatis membersihkan, membangun proyek di dalam Docker, dan menjalankan Bochs. Pada menu Bochs, tekan `Enter` untuk memulai simulasi.

---

## 4. Hasil Akhir dan Pengujian

Sistem operasi berhasil menjalankan semua skenario pengujian, membuktikan bahwa seluruh fungsionalitas telah diimplementasikan dengan benar.

![Screenshot From 2025-06-17 07-33-11](https://github.com/user-attachments/assets/a79ebaab-8c8d-430f-a407-fb31a67dbc54)

![image](https://github.com/user-attachments/assets/ef2b82bc-d9c4-48da-be4c-2760aac3cdd4)

Screenshot di atas menunjukkan bahwa perintah `echo` dan perintah berantai `echo | wc` keduanya memberikan output yang benar dan sesuai harapan. Ini menandakan bahwa seluruh komponen, dari pembacaan input, parsing, eksekusi, hingga mekanisme pipa, telah berfungsi secara sinergis.
