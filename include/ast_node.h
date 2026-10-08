#ifndef _COMPIL_AST_NODE_H_
#define _COMPIL_AST_NODE_H_

#include "defs.h"
#include "kernel.h"

// 추상구문 노드
typedef struct ast_node
{
    uint8_t node_type;

    // 위치를 가리키는 변수
    uint16_t right;
    uint16_t mid;
    uint16_t left;

    // 노드 데이터 저장
    union
    {
        uint64_t num; // 숫자 값
        uint16_t sym; // 변수 이름 ID
        uint8_t op;   // 연산자 종류
        uint8_t type; // 타입

    } value;

} ast_node;

enum NODE_TYPE
{
    NONE_NODE = 1, // 노드가 없음

    NUM_NODE,    // 정수 또는 숫자 값
    FLOAT_NODE,  // 실수 값
    STRING_NODE, // 문자열
    VAR_NODE,    // 변수
    TYPE_NODE,   // 자료형
    PTR_NODE,    // 포인터

    UNARY_OP_NODE,  // 단항 연산 (++, --, !, -, * 등)
    BINARY_OP_NODE, // 이항 연산 (+, -, *, /, ==, < 등)

    STMT_NODE,  // 하나의 문장
    DECL_NODE,  // 변수 또는 함수 선언
    EXPR_NODE,  // 값을 계산하는 식
    BLOCK_NODE, // 여러 문장을 묶은 블록 { ... }

    ASSIGN_NODE,     // 대입 (=)
    COMPARE_OP_NODE, // 비교 연산 (==, !=, <, > 등)
    LOGICAL_OP_NODE, // 논리 연산 (&&, ||, !)
    CALL_NODE,       // 함수 호출
    INDEX_NODE,      // 배열 인덱스 접근 (arr[i])
    MEMBER_NODE,     // 구조체 멤버 접근 (a.b, a->b)
    FUNCTION_NODE,   // 함수
    IF_NODE,         // if 조건문
    LOOP_NODE,       // 반복문 (for, while 등)
    RETURN_NODE,     // return 문
    PARAM_NODE,      // 함수 매개변수
    ARG_NODE,        // 함수 호출 인자
};

// 노드 생성 에러
enum PARSER_ERROR_CODE
{
    // 아직 작성하지 않음
    PARSER_OK = 1, // ACK

    NOT_SO_BRACKET, // 여는 소괄호 없음
    NOT_SC_BRACKET, // 닫는 소괄호 없음
    NOT_MO_BRACKET, // 여는 중괄호 없음
    NOT_MC_BRACKET, // 닫는 중괄호 없음
    NOT_BO_BRACKET, // 여는 대괄호 없음
    NOT_BC_BRACKET, // 닫는 대괄호 없음

    NOT_SEMI, // 세미콜론 없음

    NOT_DATA_TYPE, // 자료형이 아님
    NOT_VAR_NAME,  // 변수 이름이 아님

    NOT_OP,              // 연산자가 아님
    UNEXPECTED_TOKEN,    // 예상하지 않은 토큰
    NOT_EXPRESSION,      // 표현식이 아님
    PARSER_NODE_LIMIT,   // AST 노드 배열 한도 초과
    SYMBOL_CREATE_ERROR, // 심볼 생성 오류
    SYMBOL_SEARCH_ERROR, // 심볼 탐색 오류
    SYMBOL_SCOPE_ERROR,  // 스코프 생성 및 확인 오류
};

#endif