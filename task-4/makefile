# Compiler and Linker tools
BCC = bcc
NASM = nasm
LD = ld86

# Compiler and Linker flags
BCC_FLAGS = -ansi -c -I./include
NASM_FLAGS_BIN = -f bin
NASM_FLAGS_AS86 = -f as86
LD_FLAGS = -0 -s -d

# Directories
SRC_DIR = src
BIN_DIR = bin

# File paths
FLOPPY_IMG = $(BIN_DIR)/floppy.img
BOOTLOADER_BIN = $(BIN_DIR)/bootloader.bin
KERNEL_ASM_O = $(BIN_DIR)/kernel_asm.o
KERNEL_C_O = $(BIN_DIR)/kernel.o
STDLIB_O = $(BIN_DIR)/std_lib.o
KERNEL_BIN = $(BIN_DIR)/kernel.bin

# Object files list
OBJS = $(KERNEL_ASM_O) $(KERNEL_C_O) $(STDLIB_O)

# Default target
all: build

# Build the entire project
build: prepare bootloader stdlib kernel link
	@echo "Build complete. Floppy image is at $(FLOPPY_IMG)"

# Run the OS in Bochs
run:
	bochs -f bochsrc.txt

# Create a blank floppy disk image
prepare:
	@mkdir -p $(BIN_DIR)
	@echo "Creating floppy image..."
	@dd if=/dev/zero of=$(FLOPPY_IMG) bs=512 count=2880 >/dev/null 2>&1

# Compile the bootloader
bootloader: $(SRC_DIR)/bootloader.asm
	@echo "Compiling bootloader..."
	@$(NASM) $(NASM_FLAGS_BIN) $< -o $(BOOTLOADER_BIN)

# Compile the standard library
stdlib: $(SRC_DIR)/std_lib.c
	@echo "Compiling standard library..."
	@$(BCC) $(BCC_FLAGS) $< -o $(STDLIB_O)

# Compile the kernel C and assembly files
kernel: $(SRC_DIR)/kernel.c $(SRC_DIR)/kernel.asm
	@echo "Compiling kernel..."
	@$(BCC) $(BCC_FLAGS) $(SRC_DIR)/kernel.c -o $(KERNEL_C_O)
	@$(NASM) $(NASM_FLAGS_AS86) $(SRC_DIR)/kernel.asm -o $(KERNEL_ASM_O)

# Link all parts together into the floppy image
link: bootloader kernel stdlib
	@echo "Linking files and creating final image..."
	@$(LD) $(LD_FLAGS) -o $(KERNEL_BIN) $(OBJS)
	@dd if=$(BOOTLOADER_BIN) of=$(FLOPPY_IMG) bs=512 count=1 seek=0 conv=notrunc >/dev/null 2>&1
	@dd if=$(KERNEL_BIN) of=$(FLOPPY_IMG) bs=512 seek=1 conv=notrunc >/dev/null 2>&1

# Clean up build artifacts
clean:
	@echo "Cleaning up..."
	@rm -rf $(BIN_DIR)