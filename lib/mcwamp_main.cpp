#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <iostream>
#include <cassert>
#include "clamp-config.hxx"
/* Flag set by ‘--verbose’. */
static int verbose_flag;
static bool build_mode = false, install_mode = false;
void cxxflags(void) {
    if (!build_mode && !install_mode) {
        std::cerr << "Please specify --install or --build mode before flags\n";
        abort();
    }
    // Common options
    std::cout << "--std=c++amp";
    // clamp
    if (build_mode) {
        std::cout << " -I" CMAKE_CLAMP_INC_DIR;
        // libcxx
        std::cout << " -I" CMAKE_LIBCXX_INC;
        // GMAC options, build tree
        std::cout << " -I" CMAKE_GMAC_INC_BIN_DIR;
        std::cout << " -I" CMAKE_GMAC_INC_DIR;
    } else if (install_mode) {
        std::cerr << "Not supported yet\n";
    } else {
        assert(0 && "Unreacheable!");
    }
    std::cout << std::endl;
}

void ldflags(void) {
    if (build_mode) {
        std::cout << " -L" CMAKE_LIBCXX_LIB_DIR;
        std::cout << " -L" CMAKE_LIBCXXRT_LIB_DIR;
        std::cout << " -L" CMAKE_GMAC_LIB_DIR;
        std::cout << " -Wl,--rpath="
            CMAKE_GMAC_LIB_DIR ":"
            CMAKE_LIBCXX_LIB_DIR ":"
            CMAKE_LIBCXXRT_LIB_DIR ;
    }
    std::cout << " -lgmac-hpe -lc++ -lcxxrt -ldl -lmcwamp ";
}

int main (int argc, char **argv) {
    int c;
    while (1)
    {
        static struct option long_options[] =
        {
            /* These options set a flag. */
            {"verbose", no_argument,       &verbose_flag, 1},
            {"brief",   no_argument,       &verbose_flag, 0},
            /* These options don't set a flag.
               We distinguish them by their indices. */
            {"cxxflags", no_argument,       0, 'a'},
            {"build",    no_argument,       0, 'b'},
            {"install",  no_argument,       0, 'i'},
            {"ldflags",  no_argument,       0, 'l'},
            {"delete",  required_argument, 0, 'd'},
            {"create",  required_argument, 0, 'c'},
            {"file",    required_argument, 0, 'f'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "",
                long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;

            case 'a':   // --cxxflags
                cxxflags();
                break;
            case 'l':   // --ldflags
                ldflags();
                break;
            case 'b':
                build_mode = true;
                break;
            case 'i':
                install_mode = true;
                break;
            case 'c':
                printf ("option -c with value `%s'\n", optarg);
                break;

            case 'd':
                printf ("option -d with value `%s'\n", optarg);
                break;

            case 'f':
                printf ("option -f with value `%s'\n", optarg);
                break;

            case '?':
                /* getopt_long already printed an error message. */
                break;

            default:
                abort ();
        }
    }

    /* Instead of reporting ‘--verbose’
       and ‘--brief’ as they are encountered,
       we report the final status resulting from them. */
    if (verbose_flag)
        puts ("verbose flag is set");

    /* Print any remaining command line arguments (not options). */
    if (optind < argc)
    {
        printf ("non-option ARGV-elements: ");
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        putchar ('\n');
    }

    exit (0);
}
