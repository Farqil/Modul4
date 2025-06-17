; kernel.asm

[BITS 16]
GLOBAL _putInMemory
GLOBAL _interrupt

; Titik masuk utama, langsung lompat ke kode start
jmp start

; Fungsi-fungsi yang bisa dipanggil dari C
; void putInMemory(int segment, int address, char character)
_putInMemory:
    push bp
    mov bp,sp
    push ds
    mov ax,[bp+4]
    mov si,[bp+6]
    mov cl,[bp+8]
    mov ds,ax
    mov [si],cl
    pop ds
    pop bp
    ret

; int interrupt(int number, int AX, int BX, int CX, int DX)
_interrupt:
    push bp
    mov bp,sp
    mov ax,[bp+4]
    push ds
    mov bx,cs
    mov ds,bx
    mov si,intr
    mov [si+1],al
    pop ds
    mov ax,[bp+6]
    mov bx,[bp+8]
    mov cx,[bp+10]
    mov dx,[bp+12]

intr:   int 0x00
    mov ah,0
    pop bp
    ret

; Kode startup kernel
start:
    ; Setup stack, walaupun sudah dilakukan di bootloader,
    ; ini untuk memastikan.
    mov ax, 0x1000
    mov ss, ax
    mov ax, 0xFFF0
    mov sp, ax
    mov bp, ax

    extern _main  ; Beri tahu assembler bahwa fungsi _main ada di file C
    call _main    ; Panggil fungsi main() dari kernel.c

    ; Jika main() kembali, hentikan CPU
    cli
halt:
    hlt
    jmp halt