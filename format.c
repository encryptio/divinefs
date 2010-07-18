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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>

#include "main.h"
#include "format.h"

#define FORMAT_NBUFS 32
#define FORMAT_OFFSET_LENGTH 64
#define FORMAT_UUID_LENGTH 37
#define FORMAT_HUMANSIZE_LENGTH 20

static char format_offset_buffers[FORMAT_NBUFS][FORMAT_OFFSET_LENGTH];
static int format_offset_which = 0;

char *format_offset(exec_options *eo, off_t offset) {
    char *ret = format_offset_buffers[format_offset_which++];
    if ( format_offset_which >= FORMAT_NBUFS )
        format_offset_which = 0;

    switch ( eo->offset_format_type ) {
        case offset_format_block_count:
            if ( offset & 0x1FF )
                snprintf(ret, FORMAT_OFFSET_LENGTH, "%llu x 512-blocks + %d bytes", (long long unsigned int) offset/512, (int) (offset & 0x1FF));
            else
                snprintf(ret, FORMAT_OFFSET_LENGTH, "%llu x 512-blocks", (long long unsigned int) offset/512);
            break;

        case offset_format_byte_count:
            snprintf(ret, FORMAT_OFFSET_LENGTH, "%llu bytes", (long long unsigned int) offset);
            break;

        default:
            errx(1, "Internal error: format not specified");
    }

    return ret;
}

static char format_uuid_buffers[FORMAT_NBUFS][FORMAT_UUID_LENGTH];
static int format_uuid_which = 0;

char *format_uuid(exec_options *eo, uint8_t *uuid) {
    char *ret = format_uuid_buffers[format_uuid_which++];
    if ( format_uuid_which >= FORMAT_NBUFS )
        format_uuid_which = 0;

    snprintf(ret, FORMAT_UUID_LENGTH,
            "%02hhx%02hhx%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
            uuid[0], uuid[1], uuid[2], uuid[3],
            uuid[4], uuid[5],
            uuid[6], uuid[7],
            uuid[8], uuid[9],
            uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);

    return ret;
}

static char format_humansize_buffers[FORMAT_NBUFS][FORMAT_HUMANSIZE_LENGTH];
static int format_humansize_which = 0;

char *format_humansize(exec_options *eo, off_t size) {
    char *ret = format_humansize_buffers[format_uuid_which++];
    if ( format_humansize_which >= FORMAT_NBUFS )
        format_humansize_which = 0;

    size *= 10;

    int idx = 0;

    while ( (size >= 10000) && (idx < 4) ) {
        if ( idx == 0 )
            size /= 1024;
        else
            size /= 1024;
        idx++;
    }

    char *formats[] = { "B", "KiB", "MiB", "GiB", "TiB" };

    if ( idx == 0 ) {
        snprintf(ret, FORMAT_HUMANSIZE_LENGTH, "%lluB", (long long unsigned int) size/10);
    } else {
        snprintf(ret, FORMAT_HUMANSIZE_LENGTH, "%llu.%d%s", (long long unsigned int) (size/10), (int) (size % 10), formats[idx]);
    }

    return ret;
}

