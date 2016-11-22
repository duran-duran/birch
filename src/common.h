#ifndef __COMMON_H__
#define __COMMON_H__

#include <vector>
#include <valarray>

using data_t = double;
using DataPoint = std::valarray<data_t>;

extern int rank, procs;

#include <stdio.h>
#include <cstdarg>

#define __DEBUG__

#ifdef __DEBUG__
#define LOG(FMT, ...) logMsg(rank, FMT, __VA_ARGS__)
#else
#define LOG(...)
#endif

void logMsg(int r, const char *format, ...);

#endif // __COMMON_H__

