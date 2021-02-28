#ifndef GoL_FUNC_H
#define GoL_FUNC_H

#include <stdlib.h>
#include <sys/time.h>

/**
 * Generate a random double from min to max.
 * 
 * Please, note that RAND_MAX returns a 32 bit integer, whereas a double has 53 bits of mantissa, by IEEE-754 standard. This means that there may be many more double values left out in the specified range which are not yet covered by this implementation.
 */
double rand_double(double min, double max) {
    double range = max - min;
    double div = RAND_MAX / range;

    return min + (double) random() / div;
}

/**
 * Get the elapsed wall-clock time between two time intervals.
 *
 * @param start, end    The two time intervals.
 *
 * @return    The elapsed wall-clock time in ms.
 */
double elapsed_wtime(struct timeval start, struct timeval end) {
    return (double) ((end.tv_sec * 1000000 + end.tv_usec) \
            - (start.tv_sec * 1000000 + start.tv_usec)) / 1000;
}

#endif