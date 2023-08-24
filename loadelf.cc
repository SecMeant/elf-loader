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

    p &= ~(0xfff);

    return reinterpret_cast<void *>(p);
}

static int load_section(const mipc::finbuf &elffile, const Elf64_Shdr &shdr)
{
    switch (shdr.sh_type) {
        case SHT_PROGBITS: {
            printf(
                "Loading: %s at %p of size: %zu, perm: %c%c%c\n",
                strtab_get(shdr.sh_name),
                reinterpret_cast<void*>(shdr.sh_addr),
                shdr.sh_size,
                shdr.sh_flags & SHF_ALLOC ? 'R' : '-',
                shdr.sh_flags & SHF_WRITE ? 'W' : '-',
                shdr.sh_flags & SHF_EXECINSTR ? 'X' : '-'
            );

            /* 
             * First map the section as RW so we can memcpy the data.
             * We will later mprotect to the requested prot.
             */
            void * const sec_va = reinterpret_cast<void *>(shdr.sh_addr);
            void *p = mmap(PAGE_ADDR(sec_va), PAGE_ALIGN(shdr.sh_size), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
            if (p == MAP_FAILED) {
                fprintf(stderr, "Failed to map section: %s\n", strtab_get(shdr.sh_name));
                return 1;
            }

            memcpy(sec_va, elffile.begin() + shdr.sh_offset, shdr.sh_size);

            const int prot = (shdr.sh_flags & SHF_ALLOC ? PROT_READ : 0)
                           | (shdr.sh_flags & SHF_WRITE ? PROT_WRITE : 0)
                           | (shdr.sh_flags & SHF_EXECINSTR ? PROT_EXEC: 0);
            mprotect(PAGE_ADDR(sec_va), PAGE_ALIGN(shdr.sh_size), prot);

           break;
        }
        default:
           break;
    }

    return 0;
}

int main(int argc, char **argv)
{
    static struct option long_options[] = {
        { "file", required_argument, 0, 'f' },
        { 0, 0, 0, 0 },
    };

    int option_index = 0;

    while (1) {
        int c = getopt_long(argc, argv, "f:", long_options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 0:
                /* Should not happen - we don't have any */
                abort();

            case 'f':
                elfpath = optarg;
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

    printf("Found %u sections at %zu\n", shnum, shoff);

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

    using elf_entry_func_t = void (*)();
    auto entry = reinterpret_cast<elf_entry_func_t>(in_hdr.e_entry);
    entry();

    return 0;
}
