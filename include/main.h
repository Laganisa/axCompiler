#ifndef __COMPILER_H__
#define __COMPILER_H__

#include "kernel.h"

#define STDOUT_FD 1

static void compiler_write(const int8_t *text)
{
    axlib_write(STDOUT_FD, text, str_len(text));
}

void compiler_main(int8_t *src_ptr);

#endif
