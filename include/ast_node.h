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
    NONE_NODE = 1,
    NUM_NODE,
    FLOAT_NODE,
    STRING_NODE,
    VAR_NODE,
    TYPE_NODE,
    PTR_NODE,
    UNARY_OP_NODE,
    BINARY_OP_NODE,

    STMT_NODE,
    DECL_NODE,
    EXPR_NODE,
    BLOCK_NODE,

    ASSIGN_NODE,
    COMPARE_OP_NODE,
    LOGICAL_OP_NODE,
    CALL_NODE,
    INDEX_NODE,
    MEMBER_NODE,
    FUNCTION_NODE,
    IF_NODE,
    LOOP_NODE,
    RETURN_NODE,
    PARAM_NODE,
    ARG_NODE,
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