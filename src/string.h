#ifndef STRING_CL_H
#define STRING_CL_H

#include <stdio.h>    // core input and output functions
#include <stdlib.h>   // standard general utilities library
#include <string.h>   // string handling functions
#include <limits.h>   // macro constants of the integer types
#include <errno.h>    // error numbers

#ifdef __cplusplus
extern "C" {
#endif

void copy_string(char *dst, size_t size, const char *src);

#ifdef __cplusplus
}
#endif

#endif

