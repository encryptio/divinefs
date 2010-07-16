#include "fs/ntfs.h"

void check_ntfs(exec_options *eo, off_t offset) {
    // verify generic windows filesystem magic, always present
    uint8_t tail_magic[2] = { 0, };
    EO_READ(eo, tail_magic, offset+510, 2);
    if ( memcmp(tail_magic, "\x55\xAA", 2) != 0 )
        return;

    int64_t sector_count = read_le_int64(eo, offset+0x28);
    uint8_t sectors_per_cluster = read_uint8(eo, offset+13);
    uint64_t fs_size = sector_count*512;

    // ntfs duplicates its boot sector at the end, so make sure that has its magic too
    uint8_t magic[8] = { 0, };
    EO_READ(eo, magic,      offset+sector_count*512+0x03, 8);
    EO_READ(eo, tail_magic, offset+sector_count*512+510,  2);
    if ( memcmp(tail_magic, "\x55\xAA", 2) != 0 )
        return;
    if ( memcmp(magic, "NTFS    ", 8) != 0 )
        return;

    // make sure the mft file actually exists
    uint8_t mft_magic[4];
    int64_t mft_location = sectors_per_cluster*512*read_le_int64(eo, offset+0x30);
    if ( mft_location <= 0 )
        return;
    EO_READ(eo, mft_magic, mft_location+offset, 4);
    if ( memcmp(mft_magic, "FILE", 4) != 0 )
        return;

    switch ( eo->part_format_type ) {
        case part_format_bsdlabel:
            printf("  X: % 16lld % 16lld   NTFS\n",
                    (long long) (fs_size+512)/512,
                    (long long) offset/512);
            break;

        default:
            printf("ntfs filesystem at offset %s\n", format_offset(eo, offset));
            printf("    filesystem size %llu 512-blocks (sectors) = %llu bytes ~= %s\n",
                    (long long unsigned int) sector_count+1,
                    (long long unsigned int) fs_size+512,
                    format_humansize(eo, fs_size));
            printf("\n");
    }

    if ( !eo->skip_active ) {
        eo->skip_active = true;
        eo->skip_to = offset+fs_size+512;
    }
}

