#include "fs/ffs.h"

void check_ffs(exec_options *eo, file_state *fs, off_t offset) {
    // offset is 1KiB into the superblock due to the magic position
    uint32_t magic = read_le_uint32(fs, offset+348);

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
    fs_id[0] = read_le_uint32(fs, offset+144);
    fs_id[1] = read_le_uint32(fs, offset+148);

    bool show_mountpoint = true;
    char mountpoint[468];
    FS_READ(fs, mountpoint, offset+212, 468);

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
    uint32_t ffs1_size = read_le_uint32(fs, offset+0x24);
    uint64_t ffs2_size = read_le_uint64(fs, offset+0x438);
    uint32_t fragment_size = read_le_uint32(fs, offset+0x34);
    uint32_t basic_size = read_le_uint32(fs, offset+0x30);
    uint32_t cpg = read_le_uint32(fs, offset+0xB4);
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

    // there are two superblocks at the start of every UFS partition.
    // for UFS1, the second superblock is at +8192B
    // for UFS2, the second superblock is at +16384B
    // make sure the magic and fs_id in the superblock backup are the same.

    off_t alt_offset = offset + (ver == 1 ? 8192 : 16384);

    uint32_t fs_id2[2];
    fs_id2[0] = read_le_uint32(fs, alt_offset+144);
    fs_id2[1] = read_le_uint32(fs, alt_offset+148);

    if ( memcmp(fs_id, fs_id2, 8) != 0 )
        return;

    uint32_t magic2 = read_le_uint32(fs, alt_offset+1024+348);
    if ( magic != magic2 )
        return;

    // finally, adjust the offset to be equal to the start of the partition,
    // rather than the first superblock in the partition. again, UFS1/2 differ.

    offset -= ver == 1 ? 8192 : 65536;

    switch ( eo->part_format_type ) {
        case part_format_bsdlabel:
            // TODO: is 4.2BSD always valid?
            printf("  X: % 16lld % 16lld 4.2BSD % 6d % 5d % 4d # %s\n",
                    (long long) fs_size/512,
                    (long long) offset/512,
                    fragment_size,
                    basic_size,
                    cpg,
                    mountpoint);
            break;

        default:
            printf("ufs%d filesystem at offset %s\n", ver, format_offset(eo, offset));
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

    if ( !fs->skip_active ) {
        fs->skip_active = true;
        fs->skip_to = offset+fs_size;
    }
}

