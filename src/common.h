#ifndef __COMMON_H__
#define __COMMON_H__

#include <vector>
#include <valarray>

using data_t = double;
using DataPoint = std::valarray<data_t>;

extern int rank, procs;
#define ROOT 0

#include <stdio.h>
#include <cstdarg>

#ifdef DEBUG
#define LOG(FMT, ...) logMsg(rank, FMT, __VA_ARGS__)
#else
#define LOG(...)
#endif

void logMsg(int r, const char *format, ...);

#endif // __COMMON_H__

