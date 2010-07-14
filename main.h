#ifndef __MAIN_H__
#define __MAIN_H__

#define READ_BUFFER_SIZE (1<<20)
#define READ_BUFFER_OVERLAP 0

#include <stdbool.h>

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
    enum offset_format_type offset_format_type;
    enum part_format_type part_format_type;
} exec_options;

#endif

