#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

#include "main.h"

#include "check.h"
#include "format.h"

#include "fs/ext.h"
#include "fs/linuxswap.h"
#include "fs/ffs.h"
#include "fs/reiserfs.h"
#include "fs/btrfs.h"
#include "fs/ntfs.h"
#include "fs/fat.h"
#include "fs/hfsplus.h"

void check_buffer_all(exec_options *eo, uint8_t *buf, size_t len, off_t fileoffset) {
    for (int poss = 0; poss < len; poss += 512) {
        if ( memcmp("\x53\xEF", buf+poss+56, 2) == 0 )
            check_ext(eo, poss+fileoffset);

        if ( memcmp("SWAPSPACE2", buf+poss+512-10, 10) == 0 )
            check_linuxswap(eo, poss+fileoffset);

        if ( memcmp("\x54\x19\x01\x00", buf+poss+348, 4) == 0 ) // UFS1
            check_ffs(eo, poss+fileoffset);
        if ( memcmp("\x19\x01\x54\x19", buf+poss+348, 4) == 0 ) // UFS2
            check_ffs(eo, poss+fileoffset);

        if ( memcmp("ReIsErFs", buf+poss+0x34, 8) == 0 ) // reiserfs 3.5
            check_reiserfs(eo, poss+fileoffset);
        if ( memcmp("ReIsEr2Fs", buf+poss+0x34, 9) == 0 ) // reiserfs 3.6
            check_reiserfs(eo, poss+fileoffset);
        if ( memcmp("ReIsEr3Fs", buf+poss+0x34, 9) == 0 ) // reiserfs jr
            check_reiserfs(eo, poss+fileoffset);

        if ( memcmp("_BHRfS_M", buf+poss+0x40, 8) == 0 )
            check_btrfs(eo, poss+fileoffset);

        if ( memcmp("NTFS    ", buf+poss+0x03, 8) == 0 )
            check_ntfs(eo, poss+fileoffset);

        // FAT
        if ( (buf[poss+14] != 0 || buf[poss+15] != 0) // reserved nonzero
                && (buf[poss+16] != 0) // number of fats
                && (0xF8 <= buf[poss+21] || 0xF0 == buf[poss+21]) // "media" field
                && (buf[poss+11] + buf[poss+12]*256 >= 512) // sector_size
                && (buf[poss+11] + buf[poss+12]*256 <= 4096) // sector_size again
                )
            check_fat(eo, poss+fileoffset);

        if ( memcmp("H+", buf+poss, 2) == 0 ) // HFS+
            check_hfsplus(eo, poss+fileoffset);
        if ( memcmp("HX", buf+poss, 2) == 0 ) // HFSX
            check_hfsplus(eo, poss+fileoffset);
    }
}

