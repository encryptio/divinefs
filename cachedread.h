#ifndef __CACHED_READ_H__
#define __CACHED_READ_H__

/* Why does this exist?
 * Thank FreeBSD for not allowing unaligned reads on block devices and not
 * documenting this misfeature.
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
size_t bc_read(int fh, block_cache *bc, uint8_t *buf, off_t offset, size_t len);

// convenience wrapper for file_state (scan.h)
#define FS_READ(fs, buf, off, len) bc_read((fs)->fh, &((fs)->bc), (uint8_t *)(buf), (off), (len))

#endif

