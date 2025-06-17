# Menggunakan OS Debian (stable-slim) yang ringan dan stabil sebagai dasar.
# Di dalam lingkungan ini, 'bcc' ('dev86') tersedia dan berfungsi dengan baik.
FROM debian:stable-slim

# Menjalankan perintah di dalam Debian untuk mengupdate daftar paket
# dan menginstall semua perangkat lunak yang kita butuhkan untuk kompilasi.
# -y : Otomatis menjawab 'yes' untuk semua konfirmasi.
# --no-install-recommends : Hanya menginstall paket inti tanpa paket tambahan yang tidak perlu.
RUN apt-get update && apt-get install -y bcc nasm make build-essential --no-install-recommends

# Mengatur direktori kerja default di dalam container menjadi /lilhabos.
# Semua perintah selanjutnya akan dijalankan dari direktori ini.
WORKDIR /lilhabos

# Menyalin semua file dan folder dari direktori proyek Anda saat ini (di Arch Linux)
# ke dalam direktori kerja (/lilhabos) di dalam container.
COPY . .