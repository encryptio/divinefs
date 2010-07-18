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

#include "fs/reiserfs.h"

void check_reiserfs(exec_options *eo, file_state *fs, off_t offset) {
    enum fs_type {
        reiserfs_3_5,
        reiserfs_3_6,
        reiserfs_jr
    } fs_type;

    char magic[10];
    FS_READ(fs, magic, offset+0x34, 10);

    if ( magic[6] == 'F' )
        fs_type = reiserfs_3_5;
    else if ( magic[6] == '2' )
        fs_type = reiserfs_3_6;
    else if ( magic[6] == '3' )
        fs_type = reiserfs_jr;
    else
        errx(1, "internal error: reiserfs magic");

    uint32_t block_count    = read_le_uint32(fs, offset);
    uint32_t free_blocks    = read_le_uint32(fs, offset+0x04);
    uint32_t root_block     = read_le_uint32(fs, offset+0x08);
    uint16_t block_size     = read_le_uint16(fs, offset+0x2C);
    uint16_t journal_blocks = read_le_uint16(fs, offset+0x4A); // might be zero for "default size"

    if ( free_blocks > block_count || root_block > block_count || journal_blocks > block_count )
        return;
    if ( block_size & 0xFF )
        return;
    if ( journal_blocks < 513 && journal_blocks != 0 )
        return;

    off_t fs_size = ((off_t) block_count) * block_size;

    // nonexistent on 3.5
    uint8_t uuid[16];
    FS_READ(fs, uuid, offset+0x54, 16);

    // label at +0x64 for 16 bytes

    switch ( eo->part_format_type ) {
        default:
            printf("reiserfs %s filesystem at offset %s\n",
                    fs_type == reiserfs_3_5 ? "3.5" :
                    fs_type == reiserfs_3_6 ? "3.6" : "jr",
                    format_offset(eo, offset-65536));
            printf("    filesystem size %llu 512-blocks = %llu bytes ~= %s\n",
                    (long long unsigned int) fs_size/512,
                    (long long unsigned int) fs_size,
                    format_humansize(eo, fs_size));
            if ( fs_type != reiserfs_3_5 )
                printf("    uuid %s\n", format_uuid(eo, uuid));
            if ( eo->verbose ) {
                printf("    block size %d bytes\n", block_size);
                printf("    reiserfs block count %llu\n", (long long unsigned int) block_count);
                printf("    free blocks %llu (%d%%)\n",
                        (long long unsigned int) free_blocks,
                        (int) (((off_t)free_blocks*100)/block_count));
                if ( journal_blocks )
                    printf("    journal blocks %llu (%s)\n",
                            (long long unsigned int) journal_blocks,
                            format_humansize(eo, (off_t) journal_blocks*block_size));
            }
            printf("\n");
    }

    if ( !fs->skip_active ) {
        fs->skip_active = true;
        fs->skip_to = offset-65536+fs_size;
    }
}

