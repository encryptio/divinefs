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

#include "fs/linuxswap.h"

void check_linuxswap(exec_options *eo, file_state *fs, off_t offset) {
    offset -= 7*512; // TODO: 4k block specific

    uint8_t uuid[16];
    FS_READ(fs, uuid, offset+0x40C, 16);

    uint32_t page_count = read_le_uint32(fs, offset+0x404);
    page_count += 1;

    off_t fs_size = page_count * 4096; // TODO: 4k block specific

    switch ( eo->part_format_type ) {
        default:
            printf("linux swap v1 at offset %s\n", format_offset(eo, offset));
            printf("    filesystem size %llu 512-blocks = %llu bytes ~= %s\n",
                    (long long unsigned int) fs_size/512,
                    (long long unsigned int) fs_size,
                    format_humansize(eo, fs_size));
            printf("    uuid %s\n", format_uuid(eo, uuid));
            if ( eo->verbose )
                printf("    page count %lu\n", (long unsigned) page_count);
            printf("\n");
    }

    if ( !fs->skip_active ) {
        fs->skip_active = true;
        fs->skip_to = offset+fs_size;
    }
}

