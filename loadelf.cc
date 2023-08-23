#include <cstdio>
#include <cstdlib>

#include <getopt.h>

static const char * elffile;

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
                elffile = optarg;
                break;

            case '?':
                /* getopt_long already printed an error message. */
                break;

            default:
                abort();
        }
    }

    if (!elffile) {
        fprintf(stderr, "Elf file required\n");
        return 1;
    }

    return 0;
}
