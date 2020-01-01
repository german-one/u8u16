#include <stddef.h>
#include <math.h>
#include <time.h>
#include "basic_duration.h"


#if defined(_WIN32) || defined(__CYGWIN__) // Windows default, including MinGW and Cygwin

# ifndef _WIN32_WINNT
#   define _WIN32_WINNT 0x601
# endif
# undef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
# include <windows.h>

  double GetBasicDuration(void)
  {
    static LARGE_INTEGER frequency = {0}, previous = {0};
    LARGE_INTEGER current = {0};

    if (frequency.QuadPart == 0LL)
      QueryPerformanceFrequency(&frequency);

    QueryPerformanceCounter(&current);
    double ret_val = (previous.QuadPart == 0LL) ? (double)NAN : (current.QuadPart - previous.QuadPart) / (double)frequency.QuadPart;

    previous = current;
    return ret_val;
  }

#else // POSIX

# include <unistd.h>
# include <sys/time.h>

  double GetBasicDuration(void)
  {
    static struct timeval previous = {0};
    struct timeval current = {0};

    gettimeofday(&current, NULL);
    double ret_val = (previous.tv_sec == 0L && previous.tv_usec == 0L) ? (double)NAN : ((current.tv_sec * 1000000ULL + current.tv_usec) - (previous.tv_sec * 1000000ULL + previous.tv_usec)) / 1000000.;

    previous = current;
    return ret_val;
  }

#endif // platforms
