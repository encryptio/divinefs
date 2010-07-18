/*
 * Copyright (c) 2010 Jack Christopher Kastorff <encryptio@gmail.com>
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

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

#include "scan.h"

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

    while ( true ) {
        static struct option long_options[] = {
            { "verbose", 0, NULL, 'v' },
            { "help", 0, NULL, 'h' },
            { "start-block", 1, NULL, 's' },
            { "end-block", 1, NULL, 'e' },
            { "format", 1, NULL, 'f' },
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

    scan_file(&eo);
}

