#include "common.h"

void logMsg(int r, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    printf("Process %d%s: ", r, (r == ROOT) ? " (Root)" : "");
    vprintf(format, args);
    printf("\n");

    va_end(args);
}
