#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <err.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "main.h"

#include "check.h"

void usage(char *name) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "    %s [ -vS ] [ -s START ] [ -e END ] [ -f format ] ( file | device )\n", name);
    fprintf(stderr, "    %s -h\n", name);
    fprintf(stderr, "\n");
    fprintf(stderr, "-v: verbose\n");
    fprintf(stderr, "-S: skip interior of partitions as they're found\n");
    fprintf(stderr, "-s/-e: Start and end blocks for the search (in 512-byte blocks)\n");
    fprintf(stderr, "-f: output format. valid formats:\n");
    fprintf(stderr, "    talkative (default): spew enough information to make the partition, and as\n");
    fprintf(stderr, "                         much as possible if used with -v.\n");
    fprintf(stderr, "    bsdlabel: mostly compatible with openbsd's disklabel\n");
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
    eo.skip_partitions = false;
    eo.skip_active = false;

    while ( true ) {
        static struct option long_options[] = {
            { "verbose", 0, NULL, 'v' },
            { "help", 0, NULL, 'h' },
            { "start-block", 0, NULL, 's' },
            { "end-block", 0, NULL, 'e' },
            { "format", 0, NULL, 'f' },
            { "skip", 0, NULL, 'S' },
            { NULL, 0, NULL, 0 }
        };

        int c = getopt_long(argc, argv, "vShs:e:f:", long_options, NULL);
        if ( c == -1 )
            break;

        switch (c) {
            case 'v':
                eo.verbose = 1;
                break;

            case 'S':
                eo.skip_partitions = true;
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

            case 'f':
                if (       strcmp(optarg, "bsdlabel") == 0
                        || strcmp(optarg, "disklabel") == 0 ) {
                    eo.part_format_type = part_format_bsdlabel;
                } else if ( strcmp(optarg, "talkative") == 0 ) {
                    eo.part_format_type = part_format_talkative;
                } else {
                    usage(progname);
                    errx(1, "Bad argument for --format");
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

        if ( eo.skip_partitions && eo.skip_active ) {
            read_total = eo.skip_to-eo.start_block*512;
            eo.skip_active = 0;
        }

        lseek(eo.fh, read_total+eo.start_block*512, SEEK_SET);
    }
    if ( read_now == -1 )
        err(1, "Couldn't read from %s", eo.filename);

    if ( close(eo.fh) )
        err(1, "Couldn't close %s", eo.filename);
}

