#include "fs/btrfs.h"

void check_btrfs(exec_options *eo, file_state *fs, off_t offset) {
    uint8_t uuid[16];
    FS_READ(fs, uuid, offset+0x20, 16);

    uint8_t uuid_again[16]; // redundant information - for verification
    uint8_t uuid_dev[16];
    FS_READ(fs, uuid_dev, offset+0x10B, 16);
    FS_READ(fs, uuid_again, offset+0x11B, 16);

    if ( memcmp(uuid, uuid_again, 16) != 0 )
        return;
    
    int64_t this_super_offset_from_start = read_le_int64(fs, offset+0x30);
    if ( offset-this_super_offset_from_start < 0 )
        return;
    int64_t fs_size = read_le_int64(fs, offset+0x70);
    int64_t bytes_used = read_le_int64(fs, offset+0x78);
    if ( bytes_used > fs_size )
        return;

    int64_t num_devices = read_le_int64(fs, offset+0x88);
    if ( num_devices < 1 )
        return;

    int64_t dev_fs_size = read_le_int64(fs, offset+0xC9+0x08);
    int64_t dev_bytes_used = read_le_int64(fs, offset+0xC9+0x10);
    if ( dev_bytes_used > dev_fs_size )
        return;

    if ( dev_fs_size > fs_size )
        return;
    
    switch ( eo->part_format_type ) {
        default:
            printf("btrfs filesystem at offset %s\n", format_offset(eo, offset-this_super_offset_from_start));
            if ( num_devices > 1 || eo->verbose )
                printf("    device filesystem size %llu 512-blocks = %llu bytes ~= %s\n",
                        (long long unsigned int) dev_fs_size/512,
                        (long long unsigned int) dev_fs_size,
                        format_humansize(eo, dev_fs_size));
            printf("    total filesystem size %llu 512-blocks = %llu bytes ~= %s\n",
                    (long long unsigned int) fs_size/512,
                    (long long unsigned int) fs_size,
                    format_humansize(eo, fs_size));
            printf("    fs uuid %s\n", format_uuid(eo, uuid));
            if ( num_devices > 1 || eo->verbose ) {
                printf("    device uuid %s\n", format_uuid(eo, uuid_dev));
                printf("    device count %llu\n", (long long unsigned int) num_devices);
            }
            if ( eo->verbose ) {
                printf("    total free bytes %llu (%d%%)\n",
                        (long long unsigned int) fs_size-bytes_used,
                        (int) (((off_t) (fs_size-bytes_used)*100)/fs_size));
                printf("    device free bytes %llu (%d%%)\n",
                        (long long unsigned int) dev_fs_size-dev_bytes_used,
                        (int) (((off_t) (dev_fs_size-dev_bytes_used)*100)/dev_fs_size));
            }
            printf("\n");
    }

    if ( !fs->skip_active ) {
        fs->skip_active = true;
        fs->skip_to = offset-this_super_offset_from_start+dev_fs_size;
    }
}

