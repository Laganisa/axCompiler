#ifndef _COMPIL_TOKEN_H_
#define _COMPIL_TOKEN_H_

#include "kernel.h"

// 토큰 타입
enum TOKEN_TYPE
{
    VAL_TOKEN = 1, // 변수
    IF_TOKEN,      // if 문
    ELSE_TOKEN,
    FOR_TOKEN,   // for 문
    WHILE_TOKEN, // while 문
    RETURN_TOKEN,

    // 자료형들
    UINT8_TOKEN,  // 부호없는 문자
    UINT16_TOKEN, // 부호없는 숏
    UINT32_TOKEN, // 부호없는 정수
    UINT64_TOKEN, // 부호없는 롱

    INT8_TOKEN, // 문자
    INT16_TOKEN,
    INT32_TOKEN,
    INT64_TOKEN,

    // 괄호들
    SO_BRACKET_TOKEN,
    SC_BRACKET_TOKEN,
    MO_BRACKET_TOKEN,
    MC_BRACKET_TOKEN,

    // 숫자들
    BIN_NUMBER_TOKEN,
    DEC_NUMBER_TOKEN,
    FLOAT_NUMBER_TOKEN,
    HEX_NUMBER_TOKEN,

    // 문자들
    STRING_TOKEN,

    // 수 연산자들
    PLUS_TOKEN,
    MINU_TOKEN,
    MULTI_TOKEN,
    DIV_TOKEN,

    // 논리 연산자들
    NOT_TOKEN,
    AND_TOKEN,
    OR_TOKEN,
    XOR_TOKEN,

    // 마침표
    // 세미콜론
    SEMI_TOKEN

};

#endif