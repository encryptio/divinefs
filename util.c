#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>

#include "util.h"

uint64_t read_le_uint64(int fh, off_t offset) {
    uint8_t buf[8];
    lseek(fh, offset, SEEK_SET);
    read(fh, buf, 8);
    return ((((((((((((((uint64_t)buf[7] << 8) + buf[6]) << 8) + buf[5]) << 8) + buf[4]) << 8) + buf[3]) << 8) + buf[2]) << 8) + buf[1]) << 8) + buf[0];
}

uint32_t read_le_uint32(int fh, off_t offset) {
    uint8_t buf[4];
    lseek(fh, offset, SEEK_SET);
    read(fh, buf, 4);
    return ((((((uint32_t)buf[3] << 8) + buf[2]) << 8) + buf[1]) << 8) + buf[0];
}

uint16_t read_le_uint16(int fh, off_t offset) {
    uint8_t buf[2];
    lseek(fh, offset, SEEK_SET);
    read(fh, buf, 2);
    return (((uint16_t)buf[1]) << 8) + buf[0];
}

uint64_t read_be_uint64(int fh, off_t offset) {
    uint8_t buf[8];
    lseek(fh, offset, SEEK_SET);
    read(fh, buf, 8);
    return ((((((((((((((uint64_t)buf[0] << 8) + buf[1]) << 8) + buf[2]) << 8) + buf[3]) << 8) + buf[4]) << 8) + buf[5]) << 8) + buf[6]) << 8) + buf[7];
}

uint32_t read_be_uint32(int fh, off_t offset) {
    uint8_t buf[4];
    lseek(fh, offset, SEEK_SET);
    read(fh, buf, 4);
    return ((((((uint32_t)buf[0] << 8) + buf[1]) << 8) + buf[2]) << 8) + buf[3];
}

uint16_t read_be_uint16(int fh, off_t offset) {
    uint8_t buf[2];
    lseek(fh, offset, SEEK_SET);
    read(fh, buf, 2);
    return (((uint16_t)buf[0]) << 8) + buf[1];
}

