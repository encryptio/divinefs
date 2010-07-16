#include "fs/hfsplus.h"

// based on info from Apple TN1150

void check_hfsplus(exec_options *eo, file_state *fs, off_t offset) {
    enum fs_type {
        hfsplus,
        hfsx,
    } fs_type;

    if ( read_uint8(fs, offset+1) == '+' )
        fs_type = hfsplus;
    else if ( read_uint8(fs, offset+1) == 'X' )
        fs_type = hfsx;
    else
        errx(1, "Internal error: magic is wrong");

    uint16_t version = read_be_uint16(fs, offset+2);
    if ( version < 4 || version > 5 )
        return;

    uint32_t file_count = read_be_uint32(fs, offset+32);
    uint32_t dir_count  = read_be_uint32(fs, offset+36);
    if ( file_count == 0 )
        return;

    uint32_t blocksize = read_be_uint32(fs, offset+40);
    uint32_t totalblocks = read_be_uint32(fs, offset+44);
    uint32_t freeblocks = read_be_uint32(fs, offset+48);
    off_t fs_size = (off_t) blocksize*totalblocks;

    if ( freeblocks > totalblocks )
        return;
    if ( blocksize < 512 || blocksize > 16384 ) // blocksize=4096 has a maximum drive size of 16TiB.
        return;

    // verify the allocation file exists
    uint64_t allocation_file_size = read_be_uint64(fs, offset+112);
    if ( allocation_file_size*8 < totalblocks )
        return;

    switch ( eo->part_format_type ) {
        default:
            printf("%s at offset %s\n", fs_type == hfsplus ? "hfs+" : "hfsx", format_offset(eo, offset-1024));
            printf("    filesystem size %llu 512-blocks = %llu bytes ~= %s\n",
                    (long long unsigned int) fs_size/512,
                    (long long unsigned int) fs_size,
                    format_humansize(eo, fs_size));
            if ( eo->verbose ) {
                printf("    block size %d\n", blocksize);
                printf("    free blocks %llu/%llu (%d%%)\n",
                        (long long unsigned int) freeblocks,
                        (long long unsigned int) totalblocks,
                        (int) (((off_t) freeblocks*100)/totalblocks));
                printf("    file count %llu\n", (long long unsigned int) file_count);
                printf("    directory count %llu\n", (long long unsigned int) dir_count);
            }
            printf("\n");
    }

    if ( !fs->skip_active ) {
        fs->skip_active = true;
        fs->skip_to = offset-1024+fs_size;
    }
}

