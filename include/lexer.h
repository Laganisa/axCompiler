#ifndef __LEXER_H__
#define __LEXER_H__

#include "kernel.h"

typedef struct token
{
    uint16_t type; // 토큰 타입

    uint8_t len; // 토큰의 길이
    uint8_t col; // 몇 번째 인지
    uint32_t offset; // 오프셋

    union data
    {
        int64_t num; // 숫자 저장
        int8_t name[32]; // 최대 문자 수
    } data;
    
} token;

// 렉서 체크 튜플
typedef struct check_L
{
    uint8_t to_see : 4; // 어디를 읽어야 할지
    uint8_t error_code: 4; // 에러코드
} check_L;

check_L lexer(int8_t arr[64]);

#endif