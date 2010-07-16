#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdbool.h>
#include <fcntl.h>

#include "cachedread.h"

enum offset_format_type {
    offset_format_block_count,
    offset_format_byte_count,
};

enum part_format_type {
    part_format_talkative, // print as much information as is gleaned
    part_format_bsdlabel, // format mostly compatible with bsd's disklabel(8)
};

typedef struct exec_options {
    bool verbose;
    char *filename;
    int fh;
    block_cache bc;

    off_t start_block; // initialized to zero if not given

    bool has_end_block;
    off_t end_block;

    bool skip_partitions;
    bool skip_active;
    off_t skip_to;

    enum offset_format_type offset_format_type;
    enum part_format_type part_format_type;
} exec_options;

#endif

