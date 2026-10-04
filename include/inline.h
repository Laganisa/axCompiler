#ifndef __COMPIL_INLINE_H__
#define __COMPIL_INLINE_H__

#include <types.h>

// 문자 확인 함수
static inline uint8_t is_letter(int l)
{
    if (l >= 0x41 && l <= 0x5A)
    {
        return 1;
    }
    if (l >= 0x61 && l <= 0x7A)
    {
        return 1;
    }
    return 0;
}

// 10진수 확인 함수
static inline uint8_t is_dec_number(int n)
{
    if (n >= 48 && n <= 57)
    {
        return 1;
    }

    return 0;
}

// 2진수 확인 함수
static inline uint8_t is_bin_number(int n)
{
    if (n == '0' || n == '1')
    {
        return 1;
    }
    return 0;
}

// 16진수 확인 함수
static inline uint8_t is_hex_number(int n)
{
    if (n >= '0' && n <= '9')
    {
        return 1;
    }
    if (n >= 'A' && n <= 'F')
    {
        return 1;
    }
    return 0;
}
#endif