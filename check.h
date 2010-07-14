#ifndef __CHECK_H__
#define __CHECK_H__

#include <fcntl.h>

#include <inttypes.h>
#include <unistd.h>
#include "main.h"

void check_buffer_all(exec_options *eo, uint8_t *buf, size_t len, off_t fileoffset);

#endif

