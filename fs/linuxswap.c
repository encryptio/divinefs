#include "fs/linuxswap.h"

void check_linuxswap(exec_options *eo, off_t offset) {
    offset -= 7*512; // TODO: 4k block specific

    uint8_t uuid[16];
    lseek(eo->fh, offset+0x040C, SEEK_SET);
    read(eo->fh, uuid, 16);

    uint32_t page_count;
    lseek(eo->fh, offset+0x0404, SEEK_SET);
    read(eo->fh, &page_count, 4);
    page_count += 1;

    off_t fs_size = page_count * 4096; // TODO: 4k block specific

    switch ( eo->part_format_type ) {
        default:
            printf("linux swap v1 at offset %s\n", format_offset(eo, offset));
            printf("    filesystem size %llu 512-blocks = %llu bytes ~= %s\n",
                    (long long unsigned int) fs_size/512,
                    (long long unsigned int) fs_size,
                    format_humansize(eo, fs_size));
            printf("    uuid %s\n", format_uuid(eo, uuid));
            if ( eo->verbose )
                printf("    page count %lu\n", (long unsigned) page_count);
            printf("\n");
    }

    if ( !eo->skip_active ) {
        eo->skip_active = true;
        eo->skip_to = offset+fs_size;
    }
}

