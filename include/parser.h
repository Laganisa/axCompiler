#ifndef _COMPIL_PARSER_H_
#define _COMPIL_PARSER_H_

#include "kernel.h"
#include "defs.h"
#include "lexer.h"
#include "ast_node.h"

// 파서 확인 코드
typedef struct check_Parser
{
    uint8_t error_code;  // 파서 에러코드
    uint16_t pos;        // 방금 파싱한 함수의 배열 내 위치
    uint16_t node_pos;   // 노드 배열의 어디까지 채워져 있는지
    uint16_t psr_is_end; // 함수 종료코드

} check_Parser;

check_Parser parser(ast_node *node_arr, token *token_arr, uint16_t token_number);

// 파서 재귀 함수들
check_Parser psr_expr(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number);

check_Parser psr_stmt(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number);

check_Parser psr_decl(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number);

check_Parser psr_block(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number);

#endif