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

#include "fs/ext.h"

#define MOUNTPOINT_SIZE 64

void check_ext(exec_options *eo, file_state *fs, off_t offset) {
    uint16_t this_block_group_index = read_le_uint16(fs, offset+0x5A);
    if ( this_block_group_index != 0 )
        return;

    uint32_t block_size = 1 << (read_le_uint32(fs, offset+0x18)+10);
    if ( block_size < 1024 || block_size > 8192 )
        return;

    uint32_t block_count = read_le_uint32(fs, offset+0x04);
    uint32_t blocks_free = read_le_uint32(fs, offset+0x0C);
    if ( blocks_free > block_count )
        return;

    uint32_t inodes_count = read_le_uint32(fs, offset);
    uint32_t inodes_free  = read_le_uint32(fs, offset+0x10);
    if ( inodes_free > inodes_count )
        return;

    uint8_t uuid[16];
    FS_READ(fs, uuid, offset+0x68, 16);
    
    char mountpoint[MOUNTPOINT_SIZE];
    FS_READ(fs, mountpoint, offset+136, MOUNTPOINT_SIZE);

    // make sure the mountpoint text is printable
    int i;
    for (i = 0; i < MOUNTPOINT_SIZE; i++) {
        if ( !mountpoint[i] )
            break;
        if ( !isprint(mountpoint[i]) )
            return;
    }
    if ( i == MOUNTPOINT_SIZE )
        return;

    // figure out what version we're dealing with
    uint32_t compat_features   = read_le_uint32(fs, offset+0x5C);
    uint32_t incompat_features = read_le_uint32(fs, offset+0x60);
    uint32_t readonly_features = read_le_uint32(fs, offset+0x64);

    int ver = 2;

    // remove ext2 standard features
    compat_features   &= ~0x0038; // resize_inode,dir_index,ext_attr
    incompat_features &= ~0x0012; // meta_bg,filetype
    readonly_features &= ~0x0003; // sparse_super,large_file

    if ( compat_features & 0x0004 || incompat_features & 0x0048 ) {
        // ext3 features used
        ver = 3;
        compat_features   &= ~0x0004; // has_journal
        incompat_features &= ~0x0048; // extents,journal_dev
    }

    if ( incompat_features & 0x0280 || readonly_features & 0x0078 ) {
        // ext4 features used
        ver = 4;
        incompat_features &= ~0x0280; // flex_bg,64bit
        readonly_features &= ~0x0078; // huge_file,dir_nlink,extra_isize,gdt_csum
    }

    bool needs_journal_playback = false;

    if ( incompat_features & 0x0004 ) {
        needs_journal_playback = true;
        incompat_features &= ~0x0004; // needs journal playback
    }

    // read the "status" field
    char *status_str;
    bool clean = false;

    uint16_t status = read_le_uint16(fs, offset+0x3A);
    if ( status == 0x0001 ) {
        status_str = "clean";
        clean = true;
    } else if ( status == 0x0002 )
        status_str = "error";
    else if ( status == 0x0004 )
        status_str = "orphan";
    else
        status_str = "malformed status";

    switch ( eo->part_format_type ) {
        case part_format_bsdlabel:
            // the bsds only supports "ext2fs"
            // we mark the version here so that an error occurs if the user tries
            // to use this line without knowing what they're doing
            printf("  X: % 16lld % 16lld %s                    # %s\n",
                    (long long int) block_count*block_size/512,
                    (long long int) (offset-1024)/512,
                    ver == 3 ? "ext3fs" : ver == 4 ? "ext4fs" : "ext2fs",
                    mountpoint);
            break;

        default:
            printf("ext%d filesystem at offset %s\n", ver, format_offset(eo, offset-1024));

            if ( !clean || needs_journal_playback || eo->verbose )
                printf("    %s %s\n", status_str, needs_journal_playback ? "(needs journal playback)" : "");

            if ( *mountpoint )
                printf("    last mounted on %s\n", mountpoint);

            printf("    filesystem size %llu 512-blocks = %llu bytes ~= %s\n",
                    (long long unsigned int) block_count*block_size/512,
                    (long long unsigned int) block_count*block_size,
                    format_humansize(eo, ((off_t)block_count)*((off_t)block_size)));

            printf("    uuid %s\n", format_uuid(eo, uuid));

            if ( compat_features )
                printf("    unknown compat feature bits: 0x%x", compat_features);
            if ( incompat_features )
                printf("    unknown incompat feature bits: 0x%x", incompat_features);
            if ( readonly_features )
                printf("    unknown readonly feature buts: 0x%x", readonly_features);

            if ( eo->verbose ) {
                printf("    block size %d bytes\n", block_size);

                printf("    free blocks %llu/%llu (%d%%)\n",
                        (long long unsigned int) blocks_free,
                        (long long unsigned int) block_count,
                        (int) (((off_t) blocks_free*100)/block_count));

                printf("    free inodes %llu/%llu (%d%%)\n",
                        (long long unsigned int) inodes_free,
                        (long long unsigned int) inodes_count,
                        (int) (((off_t) inodes_free*100)/inodes_count));
            }
            printf("\n");
    }

    if ( !fs->skip_active ) {
        fs->skip_active = true;
        fs->skip_to = offset-1024+(off_t)block_count*block_size;
    }
}

