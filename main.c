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
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "    %s [ -v ] [ -s START ] [ -e END ] ( file | device )\n", name);
    fprintf(stderr, "    %s -h\n", name);
    fprintf(stderr, "\n");
    fprintf(stderr, "The start and end switches (-s/-e) take inputs measured in 512-byte blocks.\n");
    fprintf(stderr, "\n");
}

int main(int argc, char **argv) {
    char *progname = argv[0];

    // line-buffered stdout
    setvbuf(stdout, NULL, _IOLBF, 0);

    exec_options eo;
    eo.verbose = 0;
    eo.offset_format_type = offset_format_block_count;
    eo.part_format_type = part_format_talkative;
    eo.start_block = 0;
    eo.has_end_block = false;

    while ( true ) {
        static struct option long_options[] = {
            { "verbose", 0, NULL, 'v' },
            { "help", 0, NULL, 'h' },
            { "start-block", 0, NULL, 's' },
            { "end-block", 0, NULL, 'e' },
            { NULL, 0, NULL, 0 }
        };

        int c = getopt_long(argc, argv, "vhs:e:", long_options, NULL);
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

            case 's':
                eo.start_block = (off_t) strtoimax(optarg, &optarg, 10);
                if ( *optarg ) {
                    usage(progname);
                    errx(1, "Bad argument format for --start-block");
                }
                break;

            case 'e':
                eo.has_end_block = true;
                eo.end_block = (off_t) strtoimax(optarg, &optarg, 10);
                if ( *optarg ) {
                    usage(progname);
                    errx(1, "Bad argument format for --end-block");
                }
                break;

            default:
                errx(1, "Whoa.");
        }
    }

    if ( optind+1 != argc ) {
        usage(progname);
        exit(1);
    }

    eo.filename = argv[optind];

    eo.fh = open(eo.filename, O_RDONLY);
    if ( eo.fh == -1 )
        err(1, "Couldn't open %s for reading", eo.filename);

    lseek(eo.fh, eo.start_block*512, SEEK_SET);

    uint8_t buf[READ_BUFFER_SIZE];
    size_t read_now = 0;
    off_t read_total = 0;
    bool is_partial_read = eo.has_end_block && eo.end_block*512 - eo.start_block*512 < READ_BUFFER_SIZE;
    while ( (read_now = read(eo.fh, buf,
                    is_partial_read
                    ? eo.end_block*512-(read_total+eo.start_block*512)
                    : READ_BUFFER_SIZE)) > 0 ) {
        if ( read_now == -1 )
            err(1, "Couldn't read from %s", eo.filename);
        check_buffer_all(&eo, buf, read_now, read_total+eo.start_block*512);

        if ( eo.verbose )
            fprintf(stderr, "\r\033[Kread %lldMiB\r", (long long)(read_total>>20));

        read_total += read_now;

        if ( eo.has_end_block && read_total+eo.start_block*512 >= eo.end_block*512 )
            break;

        if ( eo.has_end_block
                && read_total+READ_BUFFER_SIZE+eo.start_block*512 > eo.end_block*512 )
            is_partial_read = true;

        if ( read_now == READ_BUFFER_SIZE )
            read_total -= READ_BUFFER_OVERLAP;
        lseek(eo.fh, read_total+eo.start_block*512, SEEK_SET);
    }
    if ( read_now == -1 )
        err(1, "Couldn't read from %s", eo.filename);

    if ( close(eo.fh) )
        err(1, "Couldn't close %s", eo.filename);
}

