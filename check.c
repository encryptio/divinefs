#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

#include "main.h"

#include "check.h"
#include "format.h"

#include "fs/ext.h"
#include "fs/linuxswap.h"

static off_t check_magic(off_t align, off_t smoff, char *vals, int valct, uint8_t *buf, size_t len) {
    for (int poss = smoff; poss+valct < len; poss += align) {
        if ( memcmp(vals, buf+poss, valct) == 0 ) {
            return poss-smoff;
        }
    }
    return 0;
}

void check_buffer_all(exec_options *eo, uint8_t *buf, size_t len, off_t fileoffset) {
    off_t off;
    if ( (off = check_magic(512, 56, "\x53\xEF", 2, buf, len)) > 0 )
        check_ext(eo, off+fileoffset);

    if ( (off = check_magic(512, 512-10, "SWAPSPACE2", 10, buf, len)) > 0 )
        check_linuxswap(eo, off+fileoffset);


    /* TODO: HFS+
    if ( (off = check_magic(512, 0, "\x48\x2b", 2, buf, len)) > 0 )
        check_hfsplus(eo, off+fileoffset);
    if ( (off = check_magic(512, 0, "\x48\x58", 2, buf, len)) > 0 )
        check_hfsplus(eo, off+fileoffset);
    */
}

