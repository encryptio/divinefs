#include "fs/fat.h"

void check_fat(exec_options *eo, off_t offset) {
    uint8_t bits = 0;

    // verify generic windows filesystem magic, always present
    uint8_t tail_magic[2] = { 0, };
    lseek(eo->fh, offset+510, SEEK_SET);
    read(eo->fh, tail_magic, 2);
    if ( memcmp(tail_magic, "\x55\xAA", 2) != 0 )
        return;

    uint16_t sector_size = read_le_uint16(eo->fh, offset+11);
    if ( sector_size & 0xFF )
        return;

    uint8_t sectors_per_cluster = read_uint8(eo->fh, offset+13);
    if ( sectors_per_cluster == 0 )
        return;

    uint8_t reserved_sectors = read_le_uint16(eo->fh, offset+14);
    if ( reserved_sectors == 0 )
        return;

    uint8_t fat_count = read_uint8(eo->fh, offset+16);
    if ( fat_count == 0 )
        return;

    uint32_t fat_length = read_le_uint16(eo->fh, offset+22);
    uint32_t fat32_length = read_le_uint32(eo->fh, offset+36);
    if ( fat_length == 0 ) {
        if ( fat32_length == 0 )
            return;

        bits = 32;
        fat_length = fat32_length;

        uint16_t info_sector_offset = read_le_uint16(eo->fh, offset+48);
        if ( info_sector_offset == 0 )
            info_sector_offset = 1;
        
        uint8_t magic_info_start[4];
        uint8_t magic_info_end[4];

        lseek(eo->fh, offset+info_sector_offset*512, SEEK_SET);
        read(eo->fh, magic_info_start, 4);
        lseek(eo->fh, offset+info_sector_offset*512+484, SEEK_SET);
        read(eo->fh, magic_info_end, 4);

        if ( memcmp(magic_info_start, "RRaA", 4) != 0 )
            return;
        if ( memcmp(magic_info_end,   "rrAa", 4) != 0 )
            return;
    }

    uint32_t sector_count = read_le_uint16(eo->fh, offset+19);
    if ( sector_count == 0 ) {
        sector_count = read_le_uint32(eo->fh, offset+32);
        if ( sector_count == 0 )
            return;
    }

    uint16_t dir_space = read_le_uint16(eo->fh, offset+17) * 18 / 512;;

    uint32_t total_clusters = (sector_count - reserved_sectors
                               - fat_count*fat_length - dir_space)
                            / sectors_per_cluster;
    if ( total_clusters == 0 )
        return;

    if ( !bits )
        bits = (total_clusters > 0xFF4) ? 16 : 12;

    off_t fs_size = (off_t) sector_count*sector_size;

    switch ( eo->part_format_type ) {
        default:
            printf("fat%d filesystem at offset %s\n", bits, format_offset(eo, offset));
            printf("    filesystem size %llu 512-blocks = %llu bytes ~= %s\n",
                    (long long unsigned int) fs_size/512,
                    (long long unsigned int) fs_size,
                    format_humansize(eo, fs_size));
            if ( eo->verbose ) {
                printf("    number of fats %d\n", fat_count);
                printf("    fat length %d\n", fat_length);
                printf("    sector size %d\n", sector_size);
                printf("    sectors per cluster %d\n", sectors_per_cluster);
            }
            printf("\n");
    }
}

