#ifndef _COMPIL_TOKEN_H_
#define _COMPIL_TOKEN_H_

#include "kernel.h"

// 15B
typedef struct token
{
    uint16_t token_type;

    uint8_t token_src_column;
    uint32_t token_src_line;

    union token_value_data
    {
        int64_t token_numeric_value; // 숫자
        int8_t token_name[32];       // 이름
    } token_value_data;

} token;

// 토큰 타입
enum TOKEN_TYPE
{
    VAL_TOKEN = 1, // 변수
    IF_TOKEN,      // if 문
    ELSE_TOKEN,    // else 문
    FOR_TOKEN,     // for 문
    WHILE_TOKEN,   // while 문
    RETURN_TOKEN,  // 리턴 토큰
    PTR_TOKEN,     // 포인터 토큰 여기선 고전적인 c의 * 대신 $로 포인터를 확인함

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
    BO_BRACKET_TOKEN,
    BC_BRACKET_TOKEN,

    // 숫자들
    BIN_NUMBER_TOKEN,
    DEC_NUMBER_TOKEN,
    FLOAT_NUMBER_TOKEN,
    HEX_NUMBER_TOKEN,

    // 문자들
    STRING_TOKEN,

    // 산술 연산자들
    PLUS_TOKEN,
    MINU_TOKEN,
    MULTI_TOKEN,
    DIV_TOKEN,

    // 논리 연산자들
    NOT_TOKEN,
    AND_TOKEN,
    OR_TOKEN,
    XOR_TOKEN,

    // 비교 연산자들
    EQUAL_TOKEN,
    NOT_EQUAL_TOKEN,
    LESS_TOKEN,
    LESS_EQUAL_TOKEN,
    GREATER_TOKEN,
    GREATER_EQUAL_TOKEN,
    LOGICAL_AND_TOKEN,
    LOGICAL_OR_TOKEN,

    // 대입 연산자
    ASSIGN_TOKEN,

    // 구분자
    COMMA_TOKEN,
    DOT_TOKEN,
    SEMI_TOKEN,

    // 토큰 종료
    END_TOKEN,
};

// 토큰 생성 에러
enum LEXER_ERROR_CODE
{
    LEXER_OK = 1,
    FUNC_LINE_ERROR,   // 함수 줄 초과 에러
    MAX_TOKEN_ERROR,   // 최대 토큰 에러
    NOT_SYMBOL_ERROR,  // 전방 선언 누락
    PREFIX_ERROR,      // 접두사 생략
    NUMBER_MAKE_ERROR, // 올바르지 않은 숫자 정의(구성중 탈락)
    NUMBER_VOID_ERROR, // 접두사 다음 문자가 안 옴
    NUMBER_OVER_ERROR, // 너무 많은 숫자
    NAME_OVER_ERROR,   // 이름이 토큰 저장 공간을 초과함
    STRING_MAKE_ERROR, // 문자열이 닫히지 않았거나 너무 김
};

#endif