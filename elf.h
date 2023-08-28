#pragma once

#include <string_view>

using Elf32_Addr  = uint32_t; 
using Elf32_Half  = uint16_t;
using Elf32_Off   = uint32_t;
using Elf32_Sword = int32_t;
using Elf32_Word  = uint32_t;

using Elf64_Addr   = uint64_t;
using Elf64_Off    = uint64_t;
using Elf64_Half   = uint16_t;
using Elf64_Word   = uint32_t;
using Elf64_Sword  = uint32_t;
using Elf64_Xword  = uint64_t;
using Elf64_Sxword = uint64_t;

std::string_view ELFMAG = "\x7f" "ELF";

static constexpr unsigned EI_MAG0 = 0;
static constexpr unsigned EI_MAG1 = 1;
static constexpr unsigned EI_MAG2 = 2;
static constexpr unsigned EI_MAG3 = 3;
static constexpr unsigned EI_CLASS = 4;
static constexpr unsigned EI_DATA = 5;
static constexpr unsigned EI_VERSION = 6;
static constexpr unsigned EI_OSABI = 7;
static constexpr unsigned EI_ABIVERSION = 8;
static constexpr unsigned EI_PAD = 9;
static constexpr unsigned EI_NIDENT = 16;

static constexpr unsigned ELFCLASSNONE = 0;
static constexpr unsigned ELFCLASS32 = 1;
static constexpr unsigned ELFCLASS64 = 2;

static constexpr unsigned ET_NONE = 0;
static constexpr unsigned ET_REL = 1;
static constexpr unsigned ET_EXEC = 2;
static constexpr unsigned ET_DYN = 3;
static constexpr unsigned ET_CORE = 4;
static constexpr unsigned ET_LOOS = 0xFE00;
static constexpr unsigned ET_HIOS = 0xFEFF;
static constexpr unsigned ET_LOPROC = 0xFF00;
static constexpr unsigned ET_HIPROC = 0xFFFF;

static constexpr unsigned PT_NULL = 0;
static constexpr unsigned PT_LOAD = 1;
static constexpr unsigned PT_DYNAMIC = 2;
static constexpr unsigned PT_INTERP = 3;
static constexpr unsigned PT_NOTE = 4;
static constexpr unsigned PT_SHLIB = 5;
static constexpr unsigned PT_PHDR = 6;
static constexpr unsigned PT_LOOS = 0x60000000;
static constexpr unsigned PT_HIOS = 0x6FFFFFFF;
static constexpr unsigned PT_LOPROC = 0x70000000;
static constexpr unsigned PT_HIPROC = 0x7FFFFFFF;

static constexpr unsigned PF_X = 0x1;
static constexpr unsigned PF_W = 0x2;
static constexpr unsigned PF_R = 0x4;
static constexpr unsigned PF_MASKOS = 0x00FF0000;
static constexpr unsigned PF_MASKPROC = 0xFF000000;

static constexpr unsigned SHF_WRITE = 0x1;
static constexpr unsigned SHF_ALLOC = 0x2;
static constexpr unsigned SHF_EXECINSTR = 0x4;
static constexpr unsigned SHF_MASKOS = 0x0F000000;
static constexpr unsigned SHF_MASKPROC = 0xF0000000;

static constexpr unsigned short SHN_UNDEF = 0;

typedef struct {
    unsigned char e_ident[16]; /* ELF identification */
    Elf64_Half e_type;         /* Object file type */
    Elf64_Half e_machine;      /* Machine type */
    Elf64_Word e_version;      /* Object file version */
    Elf64_Addr e_entry;        /* Entry point address */
    Elf64_Off e_phoff;         /* Program header offset */
    Elf64_Off e_shoff;         /* Section header offset */
    Elf64_Word e_flags;        /* Processor-specific flags */
    Elf64_Half e_ehsize;       /* ELF header size */
    Elf64_Half e_phentsize;    /* Size of program header entry */
    Elf64_Half e_phnum;        /* Number of program header entries */
    Elf64_Half e_shentsize;    /* Size of section header entry */
    Elf64_Half e_shnum;        /* Number of section header entries */
    Elf64_Half e_shstrndx;     /* Section name string table index */
} Elf64_Ehdr;

typedef struct {
    Elf64_Word sh_name;       /* Section name */
    Elf64_Word sh_type;       /* Section type */
    Elf64_Xword sh_flags;     /* Section attributes */
    Elf64_Addr sh_addr;       /* Virtual address in memory */
    Elf64_Off sh_offset;      /* Offset in file */
    Elf64_Xword sh_size;      /* Size of section */
    Elf64_Word sh_link;       /* Link to other section */
    Elf64_Word sh_info;       /* Miscellaneous information */
    Elf64_Xword sh_addralign; /* Address alignment boundary */
    Elf64_Xword sh_entsize;   /* Size of entries, if section has table */
} Elf64_Shdr;

typedef struct {
    Elf64_Word p_type;    /* Type of segment */
    Elf64_Word p_flags;   /* Segment attributes */
    Elf64_Off p_offset;   /* Offset in file */
    Elf64_Addr p_vaddr;   /* Virtual address in memory */
    Elf64_Addr p_paddr;   /* Reserved */
    Elf64_Xword p_filesz; /* Size of segment in file */
    Elf64_Xword p_memsz;  /* Size of segment in memory */
    Elf64_Xword p_align;  /* Alignment of segment */
} Elf64_Phdr;

typedef struct {
    Elf64_Sxword d_tag;

    union {
      Elf64_Xword d_val;
      Elf64_Addr d_ptr;
    } d_un;

} Elf64_Dyn;

static constexpr Elf64_Sxword DT_NULL         = 0;
static constexpr Elf64_Sxword DT_NEEDED       = 1;
static constexpr Elf64_Sxword DT_PLTRELSZ     = 2;
static constexpr Elf64_Sxword DT_PLTGOT       = 3;
static constexpr Elf64_Sxword DT_HASH         = 4;
static constexpr Elf64_Sxword DT_STRTAB       = 5;
static constexpr Elf64_Sxword DT_SYMTAB       = 6;
static constexpr Elf64_Sxword DT_RELA         = 7;
static constexpr Elf64_Sxword DT_RELASZ       = 8;
static constexpr Elf64_Sxword DT_RELAENT      = 9;
static constexpr Elf64_Sxword DT_STRSZ        = 10;
static constexpr Elf64_Sxword DT_SYMENT       = 11;
static constexpr Elf64_Sxword DT_INIT         = 12;
static constexpr Elf64_Sxword DT_FINI         = 13;
static constexpr Elf64_Sxword DT_SONAME       = 14;
static constexpr Elf64_Sxword DT_RPATH        = 15;
static constexpr Elf64_Sxword DT_SYMBOLIC     = 16;
static constexpr Elf64_Sxword DT_REL          = 17;
static constexpr Elf64_Sxword DT_RELSZ        = 18;
static constexpr Elf64_Sxword DT_RELENT       = 19;
static constexpr Elf64_Sxword DT_PLTREL       = 20;
static constexpr Elf64_Sxword DT_DEBUG        = 21;
static constexpr Elf64_Sxword DT_TEXTREL      = 22;
static constexpr Elf64_Sxword DT_JMPREL       = 23;
static constexpr Elf64_Sxword DT_BIND_NOW     = 24;
static constexpr Elf64_Sxword DT_INIT_ARRAY   = 25;
static constexpr Elf64_Sxword DT_FINI_ARRAY   = 26;
static constexpr Elf64_Sxword DT_INIT_ARRAYSZ = 27;
static constexpr Elf64_Sxword DT_FINI_ARRAYSZ = 28;
static constexpr Elf64_Sxword DT_LOOS         = 0x60000000;
static constexpr Elf64_Sxword DT_HIOS         = 0x6FFFFFFF;
static constexpr Elf64_Sxword DT_LOPROC       = 0x70000000;
static constexpr Elf64_Sxword DT_HIPROC       = 0x7FFFFFFF;

static constexpr Elf64_Word SHT_NULL     = 0;
static constexpr Elf64_Word SHT_PROGBITS = 1;
static constexpr Elf64_Word SHT_SYMTAB   = 2;
static constexpr Elf64_Word SHT_STRTAB   = 3;
static constexpr Elf64_Word SHT_RELA     = 4;
static constexpr Elf64_Word SHT_HASH     = 5;
static constexpr Elf64_Word SHT_DYNAMIC  = 6;
static constexpr Elf64_Word SHT_NOTE     = 7;
static constexpr Elf64_Word SHT_NOBITS   = 8;
static constexpr Elf64_Word SHT_REL      = 9;
static constexpr Elf64_Word SHT_SHLIB    = 10;
static constexpr Elf64_Word SHT_DYNSYM   = 11;
static constexpr Elf64_Word SHT_LOOS     = 0x60000000;
static constexpr Elf64_Word SHT_HIOS     = 0x6FFFFFFF;
static constexpr Elf64_Word SHT_LOPROC   = 0x70000000;
static constexpr Elf64_Word SHT_HIPROC   = 0x7FFFFFFF;

/*
typedef struct {
   unsigned char e_ident[EI_NIDENT];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
} Elf32_Shdr;
*/


