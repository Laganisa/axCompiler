#ifndef _COMPIL_PARSER_H_
#define _COMPIL_PARSER_H_

#include "kernel.h"
#include "defs.h"
#include "lexer.h"
#include "ast_node.h"

/*
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
*/

// 파서 확인 코드
typedef struct check_Parser
{
    uint8_t error_code;  // 에러코드
    uint16_t psr_is_end; // 함수 종료코드

} check_Parser;

check_Parser parser(ast_node *node_arr, token *token_arr, uint16_t token_number);

// 파서 재귀 함수들
ast_node parser_expr(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number);

ast_node parser_stmt(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number);

ast_node parser_decl(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number);

ast_node parser_block(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number);

#endif