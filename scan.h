#ifndef __CHECK_H__
#define __CHECK_H__

#include <fcntl.h>

#include <inttypes.h>
#include <unistd.h>
#include "main.h"

#include "cachedread.h"

typedef struct file_state {
    int fh;
    block_cache bc;

    bool skip_active; // TODO: is there a better place for these?
    off_t skip_to;
} file_state;

void scan_file(exec_options *eo);

#endif

