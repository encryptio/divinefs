#include "fs/ffs.h"

void check_ffs(exec_options *eo, off_t offset) {
    // offset is 1KiB into the superblock due to the magic position
    uint32_t magic = read_le_uint32(eo->fh, offset+348);

    int ver = 0;
    if ( magic == 0x00011954 )
        ver = 1;
    else if ( magic == 0x19540119 )
        ver = 2;
    else
        return;

    // now we'll point to the actual start of the partition...
    offset -= 1024;
    if ( offset < 0 )
        return;

    // read some basic information
    uint32_t fs_id[2];
    fs_id[0] = read_le_uint32(eo->fh, offset+144);
    fs_id[1] = read_le_uint32(eo->fh, offset+148);

    bool show_mountpoint = true;
    char mountpoint[468];
    lseek(eo->fh, offset+768, SEEK_SET);
    read(eo->fh, mountpoint, 468);

    // make sure the mountpoint text is printable
    int i;
    for (i = 0; i < 468; i++) {
        if ( !mountpoint[i] )
            break;
        if ( !isprint(mountpoint[i]) )
            return;
    }
    if ( i == 468 )
        return;
    if ( i == 0 )
        show_mountpoint = false;

    // filesystem size
    uint32_t ffs1_size = read_le_uint32(eo->fh, offset+0x24);
    uint64_t ffs2_size = read_le_uint64(eo->fh, offset+0x438);
    uint32_t fragment_size = read_le_uint32(eo->fh, offset+0x34);
    uint32_t basic_size = read_le_uint32(eo->fh, offset+0x30);
    uint32_t cpg = read_le_uint32(eo->fh, offset+0xB4);
    off_t fs_size;
    if ( ver == 1 )
        fs_size = ((off_t)ffs1_size)*fragment_size;
    else if ( ver == 2 )
        fs_size = ((off_t)ffs2_size)*fragment_size;
    else
        errx(1, "NOT REACHED");

    // sanity checks
    if ( fragment_size > 131072 || fragment_size < 512 || basic_size > 131072 || basic_size < 512 )
        return;
    if ( fragment_size & 0xFF || basic_size & 0xFF )
        return;

    switch ( eo->part_format_type ) {
        default:
            printf("ufs%d filesystem at offset %s\n", ver, format_offset(eo, offset-16384-(ver == 1 ? 0 : 65536)));
            if ( show_mountpoint )
                printf("    last mounted on %s\n", mountpoint);
            if ( eo->verbose )
                printf("    filesystem id %04x-%04x\n", fs_id[0], fs_id[1]);
            printf("    filesystem size %llu 512-blocks = %llu bytes ~= %s\n",
                    (long long unsigned int) fs_size/512,
                    (long long unsigned int) fs_size,
                    format_humansize(eo, fs_size));
            printf("    fsize=%d bsize=%d cpg=%d\n", fragment_size, basic_size, cpg);
            printf("\n");
    }
}

