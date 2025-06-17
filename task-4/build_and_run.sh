#!/bin/bash

# Skrip untuk membersihkan, membangun (build), dan menjalankan (run) LilHabOS secara otomatis.

echo "======================================================"
echo "==> LANGKAH 1: Membersihkan hasil build sebelumnya..."
echo "======================================================"
# Buat direktori bin jika belum ada untuk menghindari error
mkdir -p bin
rm -f bin/*

echo ""
echo "======================================================"
echo "==> LANGKAH 2: Membangun proyek di dalam Docker..."
echo "======================================================"
# Menjalankan 'make build' secara non-interaktif di dalam container
docker run --rm -v "$(pwd)":/lilhabos lilhabos-builder make build

# Memeriksa apakah proses build berhasil
if [ $? -ne 0 ]; then
    echo ""
    echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    echo "!!! BUILD GAGAL: Proses 'make build' di dalam Docker error."
    echo "!!! Silakan periksa pesan error di atas."
    echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    exit 1
else
    echo ""
    echo "*******************************************************"
    echo "*** BUILD BERHASIL: floppy.img telah berhasil dibuat."
    echo "*******************************************************"
fi

echo ""
echo "======================================================"
echo "==> LANGKAH 3: Menjalankan OS di Bochs..."
echo "======================================================"
make run