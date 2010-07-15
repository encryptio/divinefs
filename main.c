#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <err.h>
#include <inttypes.h>
#include <stdbool.h>

#include "main.h"

#include "check.h"

void usage(char *name) {
    fprintf(stderr, "Usage: %s ( file | device )+\n", name);
}

int main(int argc, char **argv) {
    char *progname = argv[0];

    // line-buffered stdout
    setvbuf(stdout, NULL, _IOLBF, 0);

    exec_options eo;
    eo.verbose = 0;
    eo.offset_format_type = offset_format_block_count;
    eo.part_format_type = part_format_talkative;

    while ( true ) {
        static struct option long_options[] = {
            { "verbose", 0, NULL, 'v' },
            { "help", 0, NULL, 'h' },
            { NULL, 0, NULL, 0 }
        };

        int c = getopt_long(argc, argv, "vh", long_options, NULL);
        if ( c == -1 )
            break;

        switch (c) {
            case 'v':
                eo.verbose = 1;
                break;
            case 'h':
                usage(progname);
                exit(1);
                break;
            default:
                errx(1, "Whoa.");
        }
    }

    if ( optind == argc ) {
        usage(progname);
        exit(1);
    }

    for (int i = optind; i < argc; i++) {
        eo.fh = open(argv[i], O_RDONLY);
        if ( eo.fh == -1 )
            err(1, "Couldn't open %s for reading", argv[i]);

        if ( eo.verbose )
            fprintf(stderr, "looking at %s\n", argv[i]);

        eo.filename = argv[i];

        uint8_t buf[READ_BUFFER_SIZE];
        size_t read_now = 0;
        off_t read_total = 0;
        while ( (read_now = read(eo.fh, buf, READ_BUFFER_SIZE)) > 0 ) {
            check_buffer_all(&eo, buf, read_now, read_total);

            if ( eo.verbose )
                fprintf(stderr, "\r\033[Kread %lldMiB\r", (long long)(read_total>>20));

            read_total += read_now;
            if ( read_now == READ_BUFFER_SIZE )
                read_total -= READ_BUFFER_OVERLAP;
            lseek(eo.fh, read_total, SEEK_SET);
        }

        if ( close(eo.fh) )
            err(1, "Couldn't close %s", argv[i]);
    }
}

