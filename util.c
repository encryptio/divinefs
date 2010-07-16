#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>

#include "util.h"

////////////////////////////////////////////////////////////////////////////////
// int le (2s complement)

int64_t read_le_int64(file_state *fs, off_t offset) {
    uint8_t buf[8];
    FS_READ(fs, buf, offset, 8);
    int64_t ret = ((((((((((((((int64_t)(buf[7] & 0x7F) << 8) + buf[6]) << 8) + buf[5]) << 8) + buf[4]) << 8) + buf[3]) << 8) + buf[2]) << 8) + buf[1]) << 8) + buf[0];
    if ( buf[7] & 0x80 )
        ret = ret - ((int64_t) 1 << 63) - ((int64_t) 1 << 63);
    return ret;
}

int32_t read_le_int32(file_state *fs, off_t offset) {
    uint8_t buf[4];
    FS_READ(fs, buf, offset, 4);
    int32_t ret = (((((((int32_t)(buf[3] & 0x7F)) << 8) + buf[2]) << 8) + buf[1]) << 8) + buf[0];
    if ( buf[3] & 0x80 )
        ret = ret - ((int32_t) 1 << 31) - ((int32_t) 1 << 31);
    return ret;
}

int16_t read_le_int16(file_state *fs, off_t offset) {
    uint8_t buf[2];
    FS_READ(fs, buf, offset, 2);
    int16_t ret = (((int16_t) (buf[1] & 0x7F)) << 8) + buf[0];
    if ( buf[1] & 0x80 )
        ret = ret - ((int16_t) 1 << 15) - ((int16_t) 1 << 15);
    return ret;
}

////////////////////////////////////////////////////////////////////////////////
// uint le

uint64_t read_le_uint64(file_state *fs, off_t offset) {
    uint8_t buf[8];
    FS_READ(fs, buf, offset, 8);
    return ((((((((((((((uint64_t)buf[7] << 8) + buf[6]) << 8) + buf[5]) << 8) + buf[4]) << 8) + buf[3]) << 8) + buf[2]) << 8) + buf[1]) << 8) + buf[0];
}

uint32_t read_le_uint32(file_state *fs, off_t offset) {
    uint8_t buf[4];
    FS_READ(fs, buf, offset, 4);
    return ((((((uint32_t)buf[3] << 8) + buf[2]) << 8) + buf[1]) << 8) + buf[0];
}

uint16_t read_le_uint16(file_state *fs, off_t offset) {
    uint8_t buf[2];
    FS_READ(fs, buf, offset, 2);
    return (((uint16_t)buf[1]) << 8) + buf[0];
}

////////////////////////////////////////////////////////////////////////////////
// uint be

uint64_t read_be_uint64(file_state *fs, off_t offset) {
    uint8_t buf[8];
    FS_READ(fs, buf, offset, 8);
    return ((((((((((((((uint64_t)buf[0] << 8) + buf[1]) << 8) + buf[2]) << 8) + buf[3]) << 8) + buf[4]) << 8) + buf[5]) << 8) + buf[6]) << 8) + buf[7];
}

uint32_t read_be_uint32(file_state *fs, off_t offset) {
    uint8_t buf[4];
    FS_READ(fs, buf, offset, 4);
    return ((((((uint32_t)buf[0] << 8) + buf[1]) << 8) + buf[2]) << 8) + buf[3];
}

uint16_t read_be_uint16(file_state *fs, off_t offset) {
    uint8_t buf[2];
    FS_READ(fs, buf, offset, 2);
    return (((uint16_t)buf[0]) << 8) + buf[1];
}

////////////////////////////////////////////////////////////////////////////////
// uint8

uint8_t read_uint8(file_state *fs, off_t offset) {
    uint8_t buf;
    FS_READ(fs, &buf, offset, 1);
    return buf;
}

int8_t read_int8(file_state *fs, off_t offset) { // 2s complement
    uint8_t buf;
    FS_READ(fs, &buf, offset, 1);
    int8_t ret = buf & 0x7F;
    if ( buf & 0x80 )
        ret = ret - ((int8_t) 1 << 7) - ((int8_t) 1 << 7);
    return ret;
}


