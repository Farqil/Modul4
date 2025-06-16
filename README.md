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
Pertama, kita perlu membuat tiga akun pengguna yang akan menjadi aktor dalam drama ini. Buka terminal dan jalankan perintah berikut dengan hak akses sudo.

### 1. Buat user:
Gunakan perintah useradd untuk membuat setiap pengguna. Opsi -m akan sekaligus membuat direktori home untuk mereka.

```bash 
sudo useradd -m DainTontas
sudo useradd -m SunnyBolt
sudo useradd -m Ryeku
```

![Screenshot From 2025-06-17 00-26-24](https://github.com/user-attachments/assets/d7d8e28b-1211-4292-bb1c-8f59b8331bba)

### 2. Atur password:
Selanjutnya, atur password untuk setiap pengguna agar mereka bisa login. Anda akan diminta untuk memasukkan dan mengkonfirmasi password baru untuk setiap akun.

``` bash
sudo passwd DainTontas
sudo passwd SunnyBolt
sudo passwd Ryeku
```

![Screenshot From 2025-06-17 00-27-56](https://github.com/user-attachments/assets/9d733b2a-206e-4bbf-aaeb-07ff0e563db5)
