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
    }

    /* TODO: HFS+
    if ( (off = check_magic(512, 0, "\x48\x2b", 2, buf, len)) > 0 )
        check_hfsplus(eo, off+fileoffset);
    if ( (off = check_magic(512, 0, "\x48\x58", 2, buf, len)) > 0 )
        check_hfsplus(eo, off+fileoffset);
    */
}

