#ifndef __COMPIL_LEXER_H__
#define __COMPIL_LEXER_H__

#include "kernel.h"
#include "defs.h"
#include "token.h"

// 15B
typedef struct token
{
    uint16_t type; // 토큰 타입

    uint8_t col;     // 몇 번째 인지
    uint32_t offset; // 오프셋

    union data
    {
        int64_t num;     // 숫자 저장
        int8_t name[32]; // 최대 문자 수
    } data;

} token;

#pragma region keywords

typedef struct keyword_t
{
    const char *name;
    enum TOKEN_TYPE type;
} keyword_t;

static keyword_t keywords[] =
    {
        {"if", IF_TOKEN},
        {"else", ELSE_TOKEN},
        {"for", FOR_TOKEN},
        {"while", WHILE_TOKEN},
        {"return", RETURN_TOKEN},

        {"uint8", UINT8_TOKEN},
        {"uint16", UINT16_TOKEN},
        {"uint32", UINT32_TOKEN},
        {"uint64", UINT64_TOKEN},

        {"int8", INT8_TOKEN},
        {"int16", INT16_TOKEN},
        {"int32", INT32_TOKEN},
        {"int64", INT64_TOKEN},

        {"(", SO_BRACKET_TOKEN},
        {")", SC_BRACKET_TOKEN},
        {"{", MO_BRACKET_TOKEN},
        {"}", MC_BRACKET_TOKEN}};

#pragma endregion

// 토큰 생성 에러
enum ERROR_CODE
{
    CAN_GO = 1,
    FUNC_LINE_ERROR,   // 함수 줄 초과 에러
    MAX_TOKEN_ERROR,   // 최대 토큰 에러
    NOT_SYMBOL_ERROR,  // 전방 선언 누락
    PREFIX_ERROR,      // 접두사 생략
    NUMBER_MAKE_ERROR, // 올바르지 않은 숫자 정의(구성중 탈락)
    NUMBER_VOID_ERROR, // 접두사 다음 문자가 안 옴
    NUMBER_OVER_ERROR, // 너무 많은 숫자
};

// 렉서 체크 튜플
typedef struct check_L
{
    uint8_t error_code;       // 에러코드
    uint16_t is_end : 1;      // 함수 종료코드
    uint16_t past_token : 15; // 어디까지 토큰 배열을 채웠는지
} check_L;

check_L lexer(token *token_arr, int8_t arr[64], uint8_t see_line, uint16_t see_token);

#endif