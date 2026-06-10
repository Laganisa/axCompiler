#ifndef __COMPILER_H__
#define __COMPILER_H__

#include "kernel.h"

// 나중에 넣기
// #include "string.h"

#define STDOUT_FD 1

static size_t str_len(const int8_t *s)
{
    size_t len = 0;

    while (s[len] != '\0')
    {
        len++;
    }

    return len;
}

static void compiler_write(const int8_t *text)
{
    axlib_write(STDOUT_FD, text, str_len(text));
}

void compiler_main(int8_t *src_ptr);

#endif
