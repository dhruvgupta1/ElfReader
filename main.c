#include "elf.h"
#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<errno.h>

char* fbuf;
size_t fsize;

int mode; // 0: 32-bit 1: 64-bit

// 32-bit
Elf32_Ehdr* e32_hdr;
Elf32_Phdr* e32_phdr;
uint32_t n32_phdr;
size_t s32_phdr;
Elf32_Shdr* e32_shdr;
uint32_t n32_shdr;
size_t s32_shdr;

// 64-bit
Elf64_Ehdr* e64_hdr;
Elf64_Phdr* e64_phdr;
uint32_t n64_phdr;
size_t s64_phdr;
Elf64_Shdr* e64_shdr;
uint32_t n64_shdr;
size_t s64_shdr;

// common
char* shstrtab;

void verify_args(int argc, char const** argv);
void load_file(char const* fname);
void read_elf();

void print_elf();

// 32-bit
void print_elf_header32(Elf32_Ehdr* hdr);
void print_program_header32(Elf32_Phdr* hdr);
void print_section_header32(Elf32_Shdr* hdr);

// 64-bit
void print_elf_header64(Elf64_Ehdr* hdr);
void print_program_header64(Elf64_Phdr* hdr);
void print_section_header64(Elf64_Shdr* hdr);

int main(int argc, char const *argv[])
{
    verify_args(argc, argv);
    load_file(argv[1]); 
    read_elf();

    print_elf();
    return 0;
}

void verify_args(int argc, char const** argv) {
    if(argc!= 2) {
        printf("Incorrect number of args passed. Check args\n");
        exit(-1);
    }
}

void load_file(char const* fname) {
    FILE* fd = fopen(fname, "r");
    if(fd == NULL) {
        printf("Unable to open file %s\n", fname);
        exit(errno);
    }
    fseek(fd, 0, SEEK_END);
    fsize = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    fbuf = (char*) malloc(fsize);
    int off = 0;
    int r;
    while((r = fread(fbuf+off, 1, 1024, fd)) > 0) {
        off+=r;
    }
    if(fsize != off) printf("!!");
    fclose(fd);
}

void read_elf() {
    e32_hdr = (Elf32_Ehdr*) fbuf;
    uint32_t magic = *((uint32_t*) e32_hdr);
    if(magic != ELF_MAGIC) {
        printf("Could not locate ELF magic. Expected: %x found: %x\n", ELF_MAGIC, magic);
        exit(-1);
    }
    if(e32_hdr->e_ident[4] == 1) mode = 0;
    else mode = 1;

    if(mode == 0) {
        n32_phdr = e32_hdr->e_phnum;
        e32_phdr = (Elf32_Phdr*) (fbuf + e32_hdr->e_phoff);
        s32_phdr = e32_hdr->e_phentsize;

        n32_shdr = e32_hdr->e_shnum;
        e32_shdr = (Elf32_Shdr*) (fbuf + e32_hdr->e_shoff);
        s32_shdr = e32_hdr->e_shentsize;

        for(int i = 0; i < n32_shdr; i++) {
            Elf32_Shdr* cshdr = (Elf32_Shdr*) ((uintptr_t) e32_shdr + i*s32_shdr);
            if(cshdr->sh_type == SHT_STRTAB) {
                shstrtab = fbuf + cshdr->sh_offset;
                // break;
            }
        }
    }
    else {
        e64_hdr = (Elf64_Ehdr*) fbuf;

        n64_phdr = e64_hdr->e_phnum;
        e64_phdr = (Elf64_Phdr*) (fbuf + e64_hdr->e_phoff);
        s64_phdr = e64_hdr->e_phentsize;
        
        n64_shdr = e64_hdr->e_shnum;
        e64_shdr = (Elf64_Shdr*) (fbuf + e64_hdr->e_shoff);
        s64_shdr = e64_hdr->e_shentsize;

        for(int i = 0; i < n64_shdr; i++) {
            Elf64_Shdr* cshdr = (Elf64_Shdr*) ((uintptr_t) e64_shdr + i*s64_shdr);
            if(cshdr->sh_type == SHT_STRTAB) {
                shstrtab = fbuf + cshdr->sh_offset;
                // break;
            }
        }
    }
}

void print_elf() {
    printf("<--ELF HEADER-->\n");
    if(mode == 0) {
        print_elf_header32(e32_hdr);
        for(int i = 0; i < n32_shdr; i++) {
            printf("Index[%02d] ", i);
            print_section_header32((Elf32_Shdr*)((uintptr_t) e32_shdr + i*s32_shdr));
        }
        printf("<--PROGRAM HEADERS-->\n");
        for(int i = 0; i < n32_phdr; i++) {
            printf("Index[%02d] ", i);
            print_program_header32((Elf32_Phdr*)((uintptr_t) e32_phdr + i*s32_phdr));
        }
    }
    else {
        print_elf_header64(e64_hdr);
        for(int i = 0; i < n64_shdr; i++) {
            printf("Index[%d] ", i);
            print_section_header64((Elf64_Shdr*)((uintptr_t) e64_shdr + i*s64_shdr));
        }
        printf("<--PROGRAM HEADERS-->\n");
        for(int i = 0; i < n64_phdr; i++) {
            printf("Index[%d] ", i);
            print_program_header64((Elf64_Phdr*)((uintptr_t) e64_phdr + i*s64_phdr));
        }
    }
}

void print_elf_header32(Elf32_Ehdr* hdr) {
    printf("Program Type: 32-bit\n");
    printf("Entry point address: %08x\n", hdr->e_entry);
    printf("Number of program headers: %d\n", hdr->e_phnum);
    printf("Number of section headers: %d\n", hdr->e_shnum);
}

void print_section_header32(Elf32_Shdr* hdr) {
    printf("[Name: %.10s] Type: %d Address: %08x Offset: %08x\n", shstrtab+hdr->sh_name, hdr->sh_type, hdr->sh_addr, hdr->sh_offset);
}

void print_program_header32(Elf32_Phdr* hdr) {
    printf("Offset: %08x VA: %08x PA: %08x FSIZE: %08x MSIZE: %08x\n", hdr->p_offset, hdr->p_vaddr, hdr->p_paddr, hdr->p_filesz, hdr->p_memsz);
}

void print_elf_header64(Elf64_Ehdr* hdr) {
    printf("Program Type: 64-bit\n");
    printf("Entry point address: %08x\n", hdr->e_entry);
    printf("Number of program headers: %d\n", hdr->e_phnum);
    printf("Number of section headers: %d\n", hdr->e_shnum);
}

void print_section_header64(Elf64_Shdr* hdr) {
    printf("[Name: %.10s] Type: %d Address: %08x Offset: %08x\n", shstrtab+hdr->sh_name, hdr->sh_type, hdr->sh_addr, hdr->sh_offset);
}

void print_program_header64(Elf64_Phdr* hdr) {
    printf("Offset: %08x VA: %08x PA: %08x FSIZE: %08x MSIZE: %08x\n", hdr->p_offset, hdr->p_vaddr, hdr->p_paddr, hdr->p_filesz, hdr->p_memsz);
}