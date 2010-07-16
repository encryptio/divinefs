#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

#include "main.h"

#include "scan.h"
#include "format.h"

#include "fs/ext.h"
#include "fs/linuxswap.h"
#include "fs/ffs.h"
#include "fs/reiserfs.h"
#include "fs/btrfs.h"
#include "fs/ntfs.h"
#include "fs/fat.h"
#include "fs/hfsplus.h"

static void check_magic_numbers(exec_options *eo, file_state *fs, uint8_t *block, off_t off) {
    if ( memcmp("\x53\xEF", block+56, 2) == 0 )
        check_ext(eo, fs, off);

    if ( memcmp("SWAPSPACE2", block+512-10, 10) == 0 )
        check_linuxswap(eo, fs, off);

    if ( memcmp("\x54\x19\x01\x00", block+348, 4) == 0 ) // UFS1
        check_ffs(eo, fs, off);
    if ( memcmp("\x19\x01\x54\x19", block+348, 4) == 0 ) // UFS2
        check_ffs(eo, fs, off);

    if ( memcmp("ReIsErFs", block+0x34, 8) == 0 ) // reiserfs 3.5
        check_reiserfs(eo, fs, off);
    if ( memcmp("ReIsEr2Fs", block+0x34, 9) == 0 ) // reiserfs 3.6
        check_reiserfs(eo, fs, off);
    if ( memcmp("ReIsEr3Fs", block+0x34, 9) == 0 ) // reiserfs jr
        check_reiserfs(eo, fs, off);

    if ( memcmp("_BHRfS_M", block+0x40, 8) == 0 )
        check_btrfs(eo, fs, off);

    if ( memcmp("NTFS    ", block+0x03, 8) == 0 )
        check_ntfs(eo, fs, off);

    if ( memcmp("\x55\xAA", block+0x1FE, 2) == 0 )
        check_fat(eo, fs, off);

    if ( memcmp("H+", block, 2) == 0 ) // HFS+
        check_hfsplus(eo, fs, off);
    if ( memcmp("HX", block, 2) == 0 ) // HFSX
        check_hfsplus(eo, fs, off);
}

void scan_file(exec_options *eo) {
    uint8_t block[512];
    int ret;
    off_t off = eo->start_block*512;

    file_state fs;
    fs.skip_active = false;
    clear_block_cache(&fs.bc);

    fs.fh = open(eo->filename, O_RDONLY);
    if ( fs.fh == -1 )
        err(1, "Couldn't open %s for reading", eo->filename);

    while ( (ret = FS_READ(&fs, block, off, 512)) > 0 ) {
        check_magic_numbers(eo, &fs, block, off);

        if ( eo->verbose && (off & 0xfffff) == 0 ) {
            if ( eo->has_end_block )
                fprintf(stderr, "\r\033[K%lld/%lld blocks (%.2f%%)\r",
                        (long long) (off/512-eo->start_block),
                        (long long) (eo->end_block-eo->start_block+1),
                        0.0); // FIXME
            else
                fprintf(stderr, "\r\033[K%lld blocks (%.1f MiB)\r",
                        (long long) (off/512-eo->start_block),
                        off/1024.0/1024.0);
        }

        off += 512;

        if ( eo->skip_partitions && fs.skip_active ) {
            off = fs.skip_to;
            fs.skip_active = 0;
        }

        if ( eo->has_end_block && off > eo->end_block*512 )
            break;
    }

    // clear the progress line
    if ( eo->verbose )
        fprintf(stderr, "\r\033[K");

    if ( close(fs.fh) )
        err(1, "Couldn't close %s", eo->filename);
}

