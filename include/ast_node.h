#ifndef _COMPIL_AST_NODE_H_
#define _COMPIL_AST_NODE_H_

#include "defs.h"
#include "kernel.h"

// 추상구문 노드
typedef struct ast_node
{
    uint8_t type;

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

    ERR_NODE = 255 // 에러 노드(에러가 나올때 이 타입임)
};

// 노드 생성 에러
enum PARSER_ERROR_CODE
{
    // 아직 작성하지 않음
    PARSER_OK = 1,
    NOT_SO_BRACKET, // 여는 소괄호 없음
    NOT_SO_BRACKET, // 닫는 소괄호 없음
    NOT_MO_BRACKET, // 여는 중괄호 없음
    NOT_MO_BRACKET, // 닫는 중괄호 없음
};

#endif