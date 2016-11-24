#include "common.h"

#include <sstream>

void logMsg(int r, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    printf("Process %d%s: ", r, (r == ROOT) ? " (Root)" : "");
    vprintf(format, args);
    printf("\n");

    va_end(args);
}

std::string pointToString(const DataPoint &point)
{
    std::stringstream pointSS;
    for (size_t i = 0; i < point.size(); ++i) {
        pointSS << " " << point[i];
    }
    return pointSS.str();
}
