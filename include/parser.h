#ifndef _COMPIL_PARSER_H_
#define _COMPIL_PARSER_H_

#include "kernel.h"
#include "defs.h"
#include "lexer.h"

typedef struct parser_Resu_stack
{
    uint8_t state;
    uint16_t token_idx;

} parser_Resu_stack;

typedef struct parser_node_buf
{
    // 파서 버퍼
    uint16_t sp; // 스택 포인터
    uint16_t current_pos;

    struct token buf[6 * MAX_PARSER_BUF];                // 15.3 KB
    struct parser_Resu_stack Resu_stack[MAX_PARSER_BUF]; // 파서 재귀 스택 500개
} parser_node_buf;

typedef struct check_P
{
    uint8_t error_code;       // 에러코드
    uint16_t is_end : 1;      // 함수 종료코드
    uint16_t read_token : 15; // 어디까지 토큰 배열을 읽었는지
} check_P;

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
        uint64_t num;  // 숫자 값
        uint16_t sym;  // 변수 이름 ID
        uint8_t op;    // 연산자 종류
        uint8_t dtype; // 타입

    } value;

} ast_node;

// 넣을 배열, 토큰 배열,
check_P parser(ast_node *node_arr, token *token_arr, uint16_t token_number);

// 파서 함수들
ast_node parser_stmt(ast_node *node_arr, token *token_arr);
ast_node parser_decl(ast_node *node_arr, token *token_arr);
ast_node parser_expr(ast_node *node_arr, token *token_arr);
ast_node parser_block(ast_node *node_arr, token *token_arr);
#endif