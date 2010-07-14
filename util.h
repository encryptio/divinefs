#ifndef __UTIL_H__
#define __UTIL_H__

#include <inttypes.h>
#include <fcntl.h>

uint64_t read_le_uint64(int fh, off_t offset);
uint64_t read_be_uint64(int fh, off_t offset);
uint32_t read_le_uint32(int fh, off_t offset);
uint32_t read_be_uint32(int fh, off_t offset);
uint16_t read_le_uint16(int fh, off_t offset);
uint16_t read_be_uint16(int fh, off_t offset);

#endif

