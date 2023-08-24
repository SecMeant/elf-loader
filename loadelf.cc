#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>

#include <mipc/file.h>

#include <getopt.h>

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include "elf.h"

static const char * elfpath;
static const char * strtab;
static Elf64_Xword strtab_size;

struct {
    /*
     * If 0 (default), program is loaded by parsing program headers.
     * If 1, program is loaded by parsing section headers.
     */
    unsigned load_from_sections : 1;

    /*
     * Don't load the program into memory, but go trough the whole process of
     * parsing the ELF file.
     */
    unsigned dry_run : 1;

    /*
     * If It's PIE, we have to pick base load address ourselves.
     */
    unsigned is_pie : 1;

    /*
     * Base virtual address of where the binary is loaded.
     */
    uintptr_t base_va;
} options;

static const char* strtab_get(Elf32_Word stroffset)
{
    if (stroffset >= strtab_size)
        return "???";

    return strtab + stroffset;
}

template <typename T>
T read_from(const void *p)
{
    T ret;

    memcpy(&ret, p, sizeof(T));

    return ret;
}

#define PAGE_ALIGN(x) static_cast<decltype(x)>(((x + 0xfffUL) & (~0xfffUL)))

void * PAGE_ADDR(void *p_)
{
    uintptr_t p = reinterpret_cast<uintptr_t>(p_);

    p &= ~(0xfffUL);

    return reinterpret_cast<void *>(p);
}

static constexpr bool is_page_aligned(uintptr_t p)
{
    return p % 4096 == 0;
}

static void* ptr_offset(void *p, size_t offset)
{
    return reinterpret_cast<char*>(p) + offset;
}

static int load_section_(const mipc::finbuf &elffile, const Elf64_Shdr &shdr)
{
    printf(
        "Loading: %s at %p of size: %zu, perm: %c%c%c\n",
        strtab_get(shdr.sh_name),
        reinterpret_cast<void*>(shdr.sh_addr),
        shdr.sh_size,
        shdr.sh_flags & SHF_ALLOC ? 'R' : '-',
        shdr.sh_flags & SHF_WRITE ? 'W' : '-',
        shdr.sh_flags & SHF_EXECINSTR ? 'X' : '-'
    );

    if (options.dry_run)
        return 0;

    if (shdr.sh_size == 0)
        return 0;

    void * const va_addr = ptr_offset(PAGE_ADDR(reinterpret_cast<void*>(shdr.sh_addr)), options.base_va);
    const size_t va_size = PAGE_ALIGN((shdr.sh_addr + shdr.sh_size) - (shdr.sh_addr & (~0xfffUL)));

    /*
     * First map the section as RW so we can memcpy the data.
     * We will later mprotect to the requested prot.
     */
    void *p = mmap(
        va_addr,
        va_size,
        PROT_READ | PROT_WRITE,
        MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
        -1,
        0
    );

    if (p == MAP_FAILED) {
        fprintf(stderr, "Failed to map section: %s\n", strtab_get(shdr.sh_name));
        return 1;
    }

    memcpy(reinterpret_cast<void*>(shdr.sh_addr + options.base_va), elffile.begin() + shdr.sh_offset, shdr.sh_size);

    const int prot = (shdr.sh_flags & SHF_ALLOC ? PROT_READ : 0)
                   | (shdr.sh_flags & SHF_WRITE ? PROT_WRITE : 0)
                   | (shdr.sh_flags & SHF_EXECINSTR ? PROT_EXEC: 0);
    mprotect(va_addr, va_size, prot);

    return 0;
}

static int load_section(const mipc::finbuf &elffile, const Elf64_Shdr &shdr)
{
    switch (shdr.sh_type) {
        case SHT_PROGBITS:
            return load_section_(elffile, shdr);
            break;
        default:
            break;
    }

    return 0;
}

static int load_sections(
        const mipc::finbuf &elffile,
        const Elf64_Off shoff,
        const Elf64_Half shent_size,
        const Elf64_Half shnum
) {
    for (auto i = 0u; i < shnum; ++i) {
        const auto shdr = read_from<Elf64_Shdr>(elffile.begin() + shoff + i * shent_size);
        printf(
            "\tSection %u:\n"
            "\t\t sh_name: %s\n"
            "\t\t sh_type: %u\n"
            "\t\t sh_flags: %zu\n"
            "\t\t sh_addr: %p\n"
            "\t\t sh_offset: %zu\n"
            "\t\t sh_size: %zu\n"
            "\t\t sh_link: %u\n"
            "\t\t sh_info: %u\n"
            "\t\t sh_addralign: %zu\n"
            "\t\t sh_entsize: %zu\n",
            i,
            strtab_get(shdr.sh_name),
            shdr.sh_type,
            shdr.sh_flags,
            reinterpret_cast<void*>(shdr.sh_addr),
            shdr.sh_offset,
            shdr.sh_size,
            shdr.sh_link,
            shdr.sh_info,
            shdr.sh_addralign,
            shdr.sh_entsize
        );

        if (load_section(elffile, shdr))
            return 1;
    }

    return 0;
}

static int load_segment_(const mipc::finbuf &elffile, const Elf64_Phdr &phdr)
{
    printf(
        "Loading segment from %zx:%zu to %p:%zu %c%c%c\n"
        ,phdr.p_offset
        ,phdr.p_filesz
        ,reinterpret_cast<void*>(phdr.p_vaddr)
        ,phdr.p_memsz
        ,phdr.p_flags & PF_R ? 'R' : '-'
        ,phdr.p_flags & PF_W ? 'W' : '-'
        ,phdr.p_flags & PF_X ? 'X' : '-'
    );

    if (options.dry_run)
        return 0;

    if (phdr.p_memsz == 0)
        return 0;

    void * const va_addr = ptr_offset(PAGE_ADDR(reinterpret_cast<void*>(phdr.p_vaddr)), options.base_va);
    const size_t va_size = PAGE_ALIGN((phdr.p_vaddr + phdr.p_memsz) - (phdr.p_vaddr & (~0xfffUL)));

    /*
     * First map the section as RW so we can memcpy the data.
     * We will later mprotect to the requested prot.
     */
    void * const pseg = mmap(
        va_addr,
        va_size,
        PROT_READ | PROT_WRITE,
        MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
        -1,
        0
    );

    if (pseg == MAP_FAILED) {
        fprintf(stderr, "Failed to mmap segment\n");
        return 1;
    }

    memcpy(reinterpret_cast<void*>(phdr.p_vaddr + options.base_va), elffile.begin() + phdr.p_offset, phdr.p_filesz);

    const int prot = (phdr.p_flags & PF_R ? PROT_READ  : 0) |
                     (phdr.p_flags & PF_W ? PROT_WRITE : 0) |
                     (phdr.p_flags & PF_X ? PROT_EXEC  : 0);
    mprotect(pseg, va_size, prot);

    return 0;
}

static int load_segment(const mipc::finbuf &elffile, const Elf64_Phdr &phdr)
{
    switch (phdr.p_type) {
        case PT_LOAD:
            return load_segment_(elffile, phdr);
            break;

        default:
            return 0;
    }

    __builtin_unreachable();
}

static int load_segments(
        const mipc::finbuf &elffile,
        const Elf64_Off phoff,
        const Elf64_Half phent_size,
        const Elf64_Half phnum
) {
    for (auto i = 0u; i < phnum; ++i) {
        const auto phdr = read_from<Elf64_Phdr>(elffile.begin() + phoff + i * phent_size);
        printf(
            "\tProgram header entry %u:\n"
            "\t\t p_type: %u\n"
            "\t\t p_flags: %u\n"
            "\t\t p_offset: %zu\n"
            "\t\t p_vaddr: %p\n"
            "\t\t p_paddr: %p\n"
            "\t\t p_filesz: %zu\n"
            "\t\t p_memsz: %zu\n"
            "\t\t p_align: %zu\n"
            ,i
            ,phdr.p_type
            ,phdr.p_flags
            ,phdr.p_offset
            ,reinterpret_cast<void*>(phdr.p_vaddr)
            ,reinterpret_cast<void*>(phdr.p_paddr)
            ,phdr.p_filesz
            ,phdr.p_memsz
            ,phdr.p_align
        );

        if (load_segment(elffile, phdr))
            return 1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    static struct option long_options[] = {
        { "file", required_argument, 0, 'f' },
        { "load-from-sections", no_argument, 0, 's' },
        { "dry-run", no_argument, 0, 'd' },
        { 0, 0, 0, 0 },
    };

    int option_index = 0;

    while (1) {
        int c = getopt_long(argc, argv, "dsf:", long_options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 0:
                /* Should not happen - we don't have any */
                abort();

            case 'f':
                elfpath = optarg;
                break;

            case 's':
                options.load_from_sections = 1;
                break;

            case 'd':
                options.dry_run = 1;
                break;

            case '?':
                /* getopt_long already printed an error message. */
                break;

            default:
                abort();
        }
    }

    if (!elfpath) {
        fprintf(stderr, "Elf file required\n");
        return 1;
    }

    auto elffile = mipc::finbuf(elfpath);

    if (!elffile) {
        fprintf(stderr, "Failed to open file: %s\n", elfpath);
        return 1;
    }

    const auto in_hdr = read_from<Elf64_Ehdr>(elffile.begin());

    const std::string_view in_mag(elffile.begin(), elffile.begin() + ELFMAG.size());
    if (in_mag != ELFMAG) {
        fprintf(stderr, "bad magic %s (%zu) != %s (%zu)\n",
                in_mag.data(), in_mag.size(),
                ELFMAG.data(), ELFMAG.size());
        return 1;
    }

    const unsigned in_class = in_hdr.e_ident[EI_CLASS];
    if (in_class != ELFCLASS64) {
        fprintf(stderr, "bad elf class: %u\n", in_class);
        return 1;
    }

    if (in_hdr.e_type == ET_DYN) {
        options.is_pie = 1;
        puts("Binary is PIE");
    }

    /* TODO check other ELF header fields */

    printf("Program entry: %p\n", (void*) in_hdr.e_entry);

    const auto phoff      = in_hdr.e_phoff;
    const auto phent_size = in_hdr.e_phentsize;
    const auto phnum      = in_hdr.e_phnum;

    printf("Found %u program headers at %zu\n", phnum, phoff);

    const auto shoff      = in_hdr.e_shoff;
    const auto shent_size = in_hdr.e_shentsize;
    const auto shnum      = in_hdr.e_shnum;

    const size_t shstrndx = in_hdr.e_shstrndx;
    if (shstrndx != SHN_UNDEF) {
        const auto strtab_shdr = read_from<Elf64_Shdr>(elffile.begin() + shoff + shent_size * shstrndx);
        strtab = elffile.begin() + strtab_shdr.sh_offset;
        strtab_size = strtab_shdr.sh_size;

        printf("strtab:\n");
        for (const char *str = strtab;
             str < strtab + strtab_size;
             str += strlen(str) + 1) {
                printf("\t%s\n", str);
        }
    }

    if (options.is_pie && options.base_va == 0)
        options.base_va = 0x7401000; // TODO: Randomize

    printf("Found %u sections at %zu\n", shnum, shoff);

    if (options.load_from_sections) {
        if (const auto ret = load_sections(elffile, shoff, shent_size, shnum); ret)
            return ret;
    } else { /* Load from program headers */
        if (const auto ret = load_segments(elffile, phoff, phent_size, phnum); ret)
            return ret;
    }

    printf("\nProgram loaded. Jumping to It's entry..\n=================================\n\n");
    if (!options.dry_run) {
        using elf_entry_func_t = void (*)();
        auto entry = reinterpret_cast<elf_entry_func_t>(in_hdr.e_entry + options.base_va);
        entry();
    }

    return 0;
}
