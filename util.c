#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>

#include "util.h"

uint32_t read_le_uint32(int fh, off_t offset) {
    uint8_t buf[4];
    lseek(fh, offset, SEEK_SET);
    read(fh, buf, 4);
    return (((((buf[3] << 8) + buf[2]) << 8) + buf[1]) << 8) + buf[0];
}

uint16_t read_le_uint16(int fh, off_t offset) {
    uint8_t buf[2];
    lseek(fh, offset, SEEK_SET);
    read(fh, buf, 2);
    return ((buf[1]) << 8) + buf[0];
}

uint32_t read_be_uint32(int fh, off_t offset) {
    uint8_t buf[4];
    lseek(fh, offset, SEEK_SET);
    read(fh, buf, 4);
    return (((((buf[0] << 8) + buf[1]) << 8) + buf[2]) << 8) + buf[3];
}

uint16_t read_be_uint16(int fh, off_t offset) {
    uint8_t buf[2];
    lseek(fh, offset, SEEK_SET);
    read(fh, buf, 2);
    return ((buf[0]) << 8) + buf[1];
}

