#ifndef GoL_H
#define GoL_H

#include <stdlib.h>
#include <unistd.h>

// Custom includes
#include "../../include/globals.h"

#include "../../include/utils/log.h"
#include "../../include/utils/func.h"
#include "../../include/utils/parse.h"

#include "../../include/life/init.h"

/**
 * Swap the memory pointers between two 1D arrays.
 */
void swap_grids(bool **old, bool **_new) {
    bool *temp = *old;

    *old  = *_new;
    *_new = temp;
}

#endif
