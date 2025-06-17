[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/V7fOtAk7)
|    NRP     |      Name      |
| :--------: | :------------: |
| 5025221000 | Student 1 Name |
| 5025221102 | Marco Marcello Hugo |

# Praktikum Modul 4 _(Module 4 Lab Work)_

</div>

### Daftar Soal _(Task List)_

- [Task 1 - FUSecure](/task-1/)

- [Task 2 - LawakFS++](/task-2/)

- [Task 3 - Drama Troll](/task-3/)

- [Task 4 - LilHabOS](/task-4/)

### Laporan Resmi Praktikum Modul 4 _(Module 4 Lab Work Report)_

Tulis laporan resmi di sini!

_Write your lab work report here!_

## Task 3
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

## Task 4
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


Screenshot di atas menunjukkan bahwa perintah `echo` dan perintah berantai `echo | wc` keduanya memberikan output yang benar dan sesuai harapan. Ini menandakan bahwa seluruh komponen, dari pembacaan input, parsing, eksekusi, hingga mekanisme pipa, telah berfungsi secara sinergis.
