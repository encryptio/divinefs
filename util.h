#ifndef __UTIL_H__
#define __UTIL_H__

#include <inttypes.h>
#include <fcntl.h>

#include "main.h"

int64_t read_le_int64(exec_options *eo, off_t offset);
int32_t read_le_int32(exec_options *eo, off_t offset);
int16_t read_le_int16(exec_options *eo, off_t offset);
int8_t  read_int8(    exec_options *eo, off_t offset);

uint64_t read_le_uint64(exec_options *eo, off_t offset);
uint64_t read_be_uint64(exec_options *eo, off_t offset);
uint32_t read_le_uint32(exec_options *eo, off_t offset);
uint32_t read_be_uint32(exec_options *eo, off_t offset);
uint16_t read_le_uint16(exec_options *eo, off_t offset);
uint16_t read_be_uint16(exec_options *eo, off_t offset);
uint8_t  read_uint8(    exec_options *eo, off_t offset);

#endif

