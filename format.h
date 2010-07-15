#ifndef __FORMAT_H__
#define __FORMAT_H__

#include <unistd.h>
#include <inttypes.h>

#include "main.h"

char *format_offset(exec_options *eo, off_t offset);
char *format_uuid(exec_options *eo, uint8_t *uuid);
char *format_humansize(exec_options *eo, off_t size);

#endif
