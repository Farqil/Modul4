[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/V7fOtAk7)
|    NRP     |      Name      |
| :--------: | :------------: |
| 5025221000 | Student 1 Name |
| 5025221000 | Student 2 Name |
| 5025221000 | Student 3 Name |

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

