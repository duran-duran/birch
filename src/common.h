#ifndef __COMMON_H__
#define __COMMON_H__

#include <vector>
#include <valarray>
#include <string>
#include <limits>
#include <iterator>

using data_t = double;
using DataPoint = std::valarray<data_t>;

extern int rank, procs;
#define ROOT 0

#include <stdio.h>
#include <cstdarg>

#define LOG(FMT, ...) logMsg(rank, FMT, ##__VA_ARGS__)

#ifdef DEBUG
#define LOG_DEBUG(FMT, ...) LOG(FMT, ##__VA_ARGS__)
#else
#define LOG_DEBUG(...)
#endif

void logMsg(int r, const char *format, ...);
std::string pointToString(const DataPoint &point);

#endif // __COMMON_H__

