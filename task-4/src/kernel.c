#include "std_lib.h"
#include "kernel.h"

// Prototipe fungsi-fungsi helper
void trim(char* str);
void itoa(int n, char* str);
void reverse(char* str);
char* strstr(char* haystack, char* needle);
void execute_command(char* command_str, char* input, char* output);


// --- FUNGSI I/O DASAR ---

void printString(char* str) {
    int i = 0;
    while (str[i] != '\0') {
        interrupt(0x10, (0x0E << 8) | str[i], 0, 0, 0);
        i++;
    }
}

void readString(char* buf) {
    int i = 0;
    char c;
    while (1) {
        int key = interrupt(0x16, 0, 0, 0, 0);
        c = (char)(key & 0xFF);

        if (c == '\r') {
            buf[i] = '\0';
            printString("\n");
            return;
        } else if (c == '\b') {
            if (i > 0) {
                i--;
                printString("\b \b");
            }
        } else {
            if (i < 127) {
                buf[i] = c;
                interrupt(0x10, (0x0E << 8) | c, 0, 0, 0);
                i++;
            }
        }
    }
}

void clearScreen() {
    interrupt(0x10, 0x0600, 0x0700, 0x0000, 0x184F);
    interrupt(0x10, 0x0200, 0, 0, 0);
}


// --- LOGIKA UTAMA KERNEL ---

int main() {
    /* Deklarasi variabel di paling atas sesuai aturan C89 */
    char buf[128];
    char* commands[10];
    char pipe_buffer[512];
    char temp_buffer[512];
    int num_commands, i;
    char* current_pos;

    clearScreen();
    printString("LilHabOS - A01\n");

    while (1) {
        printString("$> ");
        readString(buf);

        if (strlen(buf) > 0) {
            // 1. Parsing Pipa (|)
            num_commands = 0;
            current_pos = buf;
            commands[num_commands++] = current_pos;
            for (i = 0; buf[i] != '\0'; i++) {
                if (buf[i] == '|') {
                    buf[i] = '\0';
                    current_pos = &buf[i + 1];
                    commands[num_commands++] = current_pos;
                }
            }

            // 2. Eksekusi rantai perintah
            clear(pipe_buffer, 512);
            for (i = 0; i < num_commands; i++) {
                clear(temp_buffer, 512);
                execute_command(commands[i], pipe_buffer, temp_buffer);
                // PERBAIKAN KRUSIAL ADA DI BARIS DI BAWAH INI:
                strcpy(temp_buffer, pipe_buffer); // Salin dari temp_buffer (sumber) ke pipe_buffer (tujuan)
            }

            // 3. Cetak output akhir
            printString(pipe_buffer);
            if (strlen(pipe_buffer) > 0) {
                printString("\n");
            }
        }
    }
    return 0;
}


// --- LOGIKA EKSEKUSI PERINTAH ---

void execute_command(char* command_str, char* input, char* output) {
    /* Deklarasi variabel di paling atas sesuai aturan C89 */
    char* cmd;
    char* args;
    int i;
    /* Variabel untuk 'wc' */
    int lines, chars, words;
    char num_buf[16];
    int in_word_flag;

    /* Inisialisasi variabel */
    cmd = command_str;
    args = NULL;
    i = 0;

    trim(command_str);

    while (cmd[i] != ' ' && cmd[i] != '\0') i++;

    if (cmd[i] == ' ') {
        cmd[i] = '\0';
        args = &cmd[i + 1];
        trim(args);
    }

    if (strcmp(cmd, "echo")) {
        if (args != NULL) strcpy(args, output);
    } else if (strcmp(cmd, "grep")) {
        if (args != NULL && strlen(input) > 0) {
            if (strstr(input, args) != NULL) {
                strcpy(input, output);
            }
        }
    } else if (strcmp(cmd, "wc")) {
        if (strlen(input) > 0) {
            lines = 1;
            chars = strlen(input);
            words = 0;
            in_word_flag = 0; /* 0 untuk false, 1 untuk true */
            i = 0;
            while (input[i] != '\0') {
                if (input[i] == ' ' || input[i] == '\n' || input[i] == '\t') {
                    in_word_flag = 0;
                } else if (in_word_flag == 0) {
                    in_word_flag = 1;
                    words++;
                }
                i++;
            }
            itoa(lines, num_buf); strcpy(num_buf, output);
            strcat(output, " ");
            itoa(words, num_buf); strcat(output, num_buf);
            strcat(output, " ");
            itoa(chars, num_buf); strcat(output, num_buf);
        } else {
            strcpy("0 0 0", output);
        }
    } else {
        printString("Unknown command: ");
        printString(cmd);
        printString("\n");
    }
}


// --- FUNGSI-FUNGSI HELPER ---

void trim(char* str) {
    int i, begin = 0;
    int end = strlen(str) - 1;
    while (str[begin] == ' ') begin++;
    while (end >= begin && str[end] == ' ') end--;
    for (i = begin; i <= end; i++) str[i - begin] = str[i];
    str[i - begin] = '\0';
}

void itoa(int n, char* str) {
    int i = 0;
    int is_negative = 0;
    if (n == 0) { str[i++] = '0'; str[i] = '\0'; return; }
    if (n < 0) { is_negative = 1; n = -n; }
    while (n != 0) { str[i++] = mod(n, 10) + '0'; n = div(n, 10); }
    if (is_negative) str[i++] = '-';
    str[i] = '\0';
    reverse(str);
}

void reverse(char* str) {
    int i = 0, j = strlen(str) - 1;
    char temp;
    while (i < j) { temp = str[i]; str[i] = str[j]; str[j] = temp; i++; j--; }
}

char* strstr(char* haystack, char* needle) {
    int i, j;
    int found_flag;
    if (*needle == '\0') return haystack;
    for (i = 0; haystack[i] != '\0'; i++) {
        found_flag = 1;
        for (j = 0; needle[j] != '\0'; j++) {
            if (haystack[i + j] == '\0' || haystack[i + j] != needle[j]) {
                found_flag = 0;
                break;
            }
        }
        if (found_flag) {
            return &haystack[i];
        }
    }
    return NULL;
}