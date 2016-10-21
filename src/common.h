#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <mpi.h>
#include <vector>
#include <valarray>

using data_t = double;
#define MPI_DATA_T MPI_DOUBLE
using DataPoint = std::valarray<data_t>;

#endif // __COMMON_H__

