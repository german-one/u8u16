// Wrapper for the related functions of duration measuring in the Windows API or in the POSIX lib.

#ifndef BASIC_DURATION_H_INCLUDED__
#define BASIC_DURATION_H_INCLUDED__

#include <math.h>
#ifdef __cplusplus
extern "C"
#endif

// Returns the time elapsed between two calls of this function in seconds.
// The first call returns NaN.
double GetBasicDuration(void);

#endif // BASIC_DURATION_H_INCLUDED__
