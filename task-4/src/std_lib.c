#include "std_lib.h"

// Fungsi pembagian integer
int div(int a, int b) {
    int result = 0;
    int sign = 1;
    if (a < 0) {
        a = -a;
        sign = -sign;
    }
    if (b < 0) {
        b = -b;
        sign = -sign;
    }
    while (a >= b) {
        a -= b;
        result++;
    }
    return result * sign;
}

// Fungsi modulo
int mod(int a, int b) {
    // Menggunakan rumus a % b = a - (a / b) * b
    return a - div(a, b) * b;
}

// Menyalin memori dari sumber ke tujuan
void memcpy(byte* src, byte* dst, unsigned int size) {
    unsigned int i;
    for (i = 0; i < size; i++) {
        dst[i] = src[i];
    }
}

// Menghitung panjang string
unsigned int strlen(char* str) {
    unsigned int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Membandingkan dua string
bool strcmp(char* str1, char* str2) {
    unsigned int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) {
            return false;
        }
        i++;
    }
    return str1[i] == str2[i];
}

// Menyalin string dari sumber ke tujuan
void strcpy(char* src, char* dst) {
    unsigned int i = 0;
    while (src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

// Membersihkan buffer (mengisi dengan nol)
void clear(byte* buf, unsigned int size) {
    unsigned int i;
    for (i = 0; i < size; i++) {
        buf[i] = 0;
    }
}

// Menambahkan string sumber ke akhir string tujuan
void strcat(char* dst, char* src) {
    unsigned int dst_len = strlen(dst);
    unsigned int i = 0;
    while (src[i] != '\0') {
        dst[dst_len + i] = src[i];
        i++;
    }
    dst[dst_len + i] = '\0';
}