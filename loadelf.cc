#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <string_view>

#include <mipc/file.h>

#include <getopt.h>

#include "elf.h"

static const char * elfpath;
// static const char * strtab;

template <typename T>
T read_from(const void *p)
{
    T ret;

    memcpy(&ret, p, sizeof(T));

    return ret;
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

    /* Not sure if it works
     *
     * const size_t shstrndx = in_hdr.e_shstrndx;
     * if (shstrndx != SHN_UNDEF) {
     *     strtab = elffile.begin() + shoff + shent_size * shstrndx;
     *     printf("strtab: %s\n", strtab);
     * }
     *
     */

    printf("Found %u sections at %zu\n", shnum, shoff);

    for (auto i = 0u; i < shnum; ++i) {
        const auto shdr = read_from<Elf64_Shdr>(elffile.begin() + shoff + i * shent_size);
        printf(
            "\tSection %u:\n"
            "\t\t sh_name: %u\n"
            "\t\t sh_type: %u\n"
            "\t\t sh_flags: %zu\n"
            "\t\t sh_addr: %zu\n"
            "\t\t sh_offset: %zu\n"
            "\t\t sh_size: %zu\n"
            "\t\t sh_link: %u\n"
            "\t\t sh_info: %u\n"
            "\t\t sh_addralign: %zu\n"
            "\t\t sh_entsize: %zu\n",
            i,
            shdr.sh_name,
            shdr.sh_type,
            shdr.sh_flags,
            shdr.sh_addr,
            shdr.sh_offset,
            shdr.sh_size,
            shdr.sh_link,
            shdr.sh_info,
            shdr.sh_addralign,
            shdr.sh_entsize
        );
    }

    return 0;
}
