#ifndef __DEFS_H__
#define __DEFS_H__

// 컴파일러 
#define MAX_SOURCE_LINE 0x3FFF
#define MAX_TOKEN_ARR 7877 // 최대 토큰 수
#define MAX_PARSER_BUF 128
#define END_LEXER 0x24 // '$'을 붙여준다

// 토큰 타입
enum TOKEN_TYPE {
    VAL_TOKEN = 1, // 변수
    IF_TOKEN, // if 문
    ELSE_TOKEN,
    FOR_TOKEN, // for 문
    WHILE_TOKEN, // while 문
    RETURN_TOKEN,

    // 자료형들
    UINT8_TOKEN, // 부호없는 문자
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
    XOR_TOKEN

};

// 토큰 생성 에러
enum ERROR_CODE {
    CAN_GO = 1,
    FUNC_LINE_ERROR,    // 함수 줄 초과 에러
    MAX_TOKEN_ERROR,        // 최대 토큰 에러
    NOT_SYMBOL_ERROR,       // 전방 선언 누락
    PREFIX_ERROR,           // 접두사 생략
    NUMBER_MAKE_ERROR,      // 올바르지 않은 숫자 정의(구성중 탈락)
    NUMBER_VOID_ERROR,     // 접두사 다음 문자가 안 옴
    NUMBER_OVER_ERROR,     // 너무 많은 숫자
    
};

#endif