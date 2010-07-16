#ifndef __UTIL_H__
#define __UTIL_H__

#include <inttypes.h>
#include <fcntl.h>

#include "scan.h"

int64_t read_le_int64(file_state *fs, off_t offset);
int32_t read_le_int32(file_state *fs, off_t offset);
int16_t read_le_int16(file_state *fs, off_t offset);
int8_t  read_int8(    file_state *fs, off_t offset);

uint64_t read_le_uint64(file_state *fs, off_t offset);
uint64_t read_be_uint64(file_state *fs, off_t offset);
uint32_t read_le_uint32(file_state *fs, off_t offset);
uint32_t read_be_uint32(file_state *fs, off_t offset);
uint16_t read_le_uint16(file_state *fs, off_t offset);
uint16_t read_be_uint16(file_state *fs, off_t offset);
uint8_t  read_uint8(    file_state *fs, off_t offset);

#endif

