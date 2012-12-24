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

#ifndef __CACHED_READ_H__
#define __CACHED_READ_H__

/* Why does this exist?
 * Thank FreeBSD for not allowing unaligned reads on block devices and not
 * documenting this misfeature.
 *
 * bug report:
 * docs/91149: read(2) can return EINVAL for unaligned access to block devices
 * http://www.freebsd.org/cgi/query-pr.cgi?pr=91149
 */

#include <inttypes.h>
#include <fcntl.h>

#define CACHE_BLOCK_SIZE ((off_t) 0x8000)
#define CACHE_BLOCK_MASK ((off_t) 0x7FFF)

#define CACHE_BLOCK_COUNT 16

typedef struct block_cache {
    uint8_t data[CACHE_BLOCK_COUNT][CACHE_BLOCK_SIZE];
    off_t   loc[ CACHE_BLOCK_COUNT];
    int next;
} block_cache;

void clear_block_cache(block_cache *bc);
ssize_t bc_read(int fh, block_cache *bc, uint8_t *buf, off_t offset, size_t len);

// convenience wrapper for file_state (scan.h)
#define FS_READ(fs, buf, off, len) bc_read((fs)->fh, &((fs)->bc), (uint8_t *)(buf), (off), (len))

#endif

