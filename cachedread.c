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
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

#include "cachedread.h"

void clear_block_cache(block_cache *bc) {
    for (int i = 0; i < CACHE_BLOCK_COUNT; i++)
        bc->loc[i] = -1;
    bc->next = 0;
}

// returns len on success
// on an error in lseek or read, returns -1 and prints a warning message
// on eof, returns 0 and sets the buf to all zero bytes
// FIXME: partial reads return len, even though the data actually put in buf is less

size_t bc_read(int fh, block_cache *bc, uint8_t *buf, off_t offset, size_t len) {
    off_t blockid = offset & ~CACHE_BLOCK_MASK;
    off_t off_in_block = offset & CACHE_BLOCK_MASK;

    if ( len + off_in_block > CACHE_BLOCK_SIZE ) {
        // read is too big for one block, do it in chunks.

        size_t ret = 0;

        size_t size_in_first_block = CACHE_BLOCK_SIZE-off_in_block;
        size_t size_in_last_block  = (offset+len) & CACHE_BLOCK_MASK;

        off_t last_blockid = (offset+len) & ~CACHE_BLOCK_MASK;

        size_t thisret = bc_read(fh, bc, buf, offset, size_in_first_block);
        if ( thisret <= 0 )
            return thisret;
        ret += thisret;

        for (off_t i = blockid+CACHE_BLOCK_SIZE; i < last_blockid; i += CACHE_BLOCK_SIZE) {
            thisret = bc_read(fh, bc, buf+(i-offset), i, CACHE_BLOCK_SIZE); // TODO: test this line
            if ( thisret < 0 )
                return thisret;
            if ( thisret == 0 )
                return ret;
            ret += thisret;
        }

        thisret = bc_read(fh, bc, buf+(last_blockid-offset), last_blockid, size_in_last_block);
        if ( thisret < 0 )
            return thisret;
        ret += thisret;

        return ret;
    }

    for (int i = 0; i < CACHE_BLOCK_COUNT; i++) {
        if ( bc->loc[i] == blockid ) {
            memcpy(buf, bc->data[i]+off_in_block, len);
            return len;
        }
    }

    bc->loc[bc->next] = blockid;
    memset(bc->data[bc->next], 0, CACHE_BLOCK_SIZE);
    
    off_t ret = lseek(fh, blockid, SEEK_SET);
    if ( ret < 0 ) {
        warn("seek failure");
        return -1;
    }

    ret = read(fh, bc->data[bc->next], CACHE_BLOCK_SIZE);
    if ( ret < 0 ) {
        warn("read failure");
        return -1;
    }

    memcpy(buf, bc->data[bc->next]+off_in_block, len);

    if ( ret == 0 ) // EOF
        return 0;

    if ( ++bc->next >= CACHE_BLOCK_COUNT )
        bc->next = 0;

    return len;
}

