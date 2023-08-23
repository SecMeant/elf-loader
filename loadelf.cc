#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include <mipc/file.h>

#include <getopt.h>

static const char * elfpath;

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

    printf("%s\n", elffile.data());

    return 0;
}
