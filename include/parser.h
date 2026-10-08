#ifndef _COMPIL_PARSER_H_
#define _COMPIL_PARSER_H_

#include "kernel.h"
#include "defs.h"
#include "lexer.h"
#include "ast_node.h"
#include "symbol.h"

// 파서 확인 코드
typedef struct check_Parser
{
    uint8_t error_code;  // 파서 에러코드
    uint16_t pos;        // 방금 파싱한 함수의 배열 내 위치
    uint16_t node_pos;   // 노드 배열의 어디까지 채워져 있는지
    uint16_t psr_is_end; // 함수 종료코드

} check_Parser;

// node_arr는 MAX_PARSER_BUF개 이상의 노드를 저장할 수 있어야 함
check_Parser parser(ast_node *node_arr, token *token_arr, uint16_t token_number);

// 처음 파싱할 때는 1, 재귀할 때는 다음 연산자 우선순위를 넘긴다
check_Parser psr_expr(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number,
    uint8_t min_precedence);
check_Parser psr_decl(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number);
check_Parser psr_block(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number,
    uint8_t function_body);
check_Parser psr_stmt(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number);

extern uint16_t global_node_pos;
extern scope *current_scope;

#define PSR_NO_NODE 0xFFFFu

static inline uint16_t psr_token_type_at(
    const token *token_arr,
    uint16_t token_pos,
    uint16_t token_number)
{
    if (token_arr == 0 || token_pos >= token_number)
    {
        return END_TOKEN;
    }

    return token_arr[token_pos].token_type;
}

static inline check_Parser psr_result(uint8_t error_code, uint16_t pos)
{
    // 리턴할 파서 상태
    check_Parser result = {0};
    result.error_code = error_code;
    result.pos = pos;
    result.node_pos = global_node_pos;
    return result;
}

static inline void psr_init_node(ast_node *node_ptr, uint8_t node_type)
{
    node_ptr->node_type = node_type;
    node_ptr->left = PSR_NO_NODE;
    node_ptr->mid = PSR_NO_NODE;
    node_ptr->right = PSR_NO_NODE;
    node_ptr->value.num = 0;
}

static inline check_Parser psr_store_node(
    ast_node *node_arr,
    ast_node node)
{
    if (node_arr == 0)
    {
        return psr_result(UNEXPECTED_TOKEN, PSR_NO_NODE);
    }

    if (global_node_pos >= MAX_PARSER_BUF)
    {
        return psr_result(PARSER_NODE_LIMIT, PSR_NO_NODE);
    }

    // 생성된 new_node를 배열에 넣기
    uint16_t npos = global_node_pos;
    node_arr[npos] = node;
    global_node_pos++;
    // 값을 넣고
    return psr_result(PARSER_OK, npos);
}

static inline void psr_symbol_name(
    const token *token_arr,
    uint16_t token_pos,
    char *name)
{
    uint16_t name_index = 0;
    while (name_index < 31 &&
           token_arr[token_pos].token_value_data.token_name[name_index] != '\0')
    {
        name[name_index] =
            (char)token_arr[token_pos].token_value_data.token_name[name_index];
        name_index++;
    }
    name[name_index] = '\0';
}

static inline uint8_t psr_symbol_create(
    const token *token_arr,
    uint16_t token_pos,
    uint16_t *symbol_id)
{
    if (token_arr == 0 || current_scope == 0 || symbol_id == 0)
    {
        return SYMBOL_SCOPE_ERROR;
    }

    char name[32] = {0};
    psr_symbol_name(token_arr, token_pos, name);
    uint8_t symbol_pos = symbol_crate(current_scope, name);
    if (symbol_pos == SYMBOL_CRATE_ERR)
    {
        return SYMBOL_CREATE_ERROR;
    }

    *symbol_id = symbol_pos;
    return PARSER_OK;
}

static inline uint8_t psr_symbol_search(
    const token *token_arr,
    uint16_t token_pos,
    uint16_t *symbol_id)
{
    if (token_arr == 0 || current_scope == 0 || symbol_id == 0)
    {
        return SYMBOL_SCOPE_ERROR;
    }

    char name[32] = {0};
    psr_symbol_name(token_arr, token_pos, name);
    uint8_t symbol_pos = symbol_search(current_scope, name);
    if (symbol_pos == SYMBOL_SEARCH_ERR)
    {
        return SYMBOL_SEARCH_ERROR;
    }

    uint64_t name_hash = axlib_fnv1a_hash_64(name);
    // 같은 이름이면 가까운 스코프의 심볼을 우선 사용
    for (scope *now_scope = current_scope;
         now_scope != 0;
         now_scope = now_scope->par)
    {
        for (uint8_t name_index = 0;
             name_index < now_scope->num;
             name_index++)
        {
            if (now_scope->symbol_buf[name_index] == name_hash)
            {
                *symbol_id = name_index;
                return PARSER_OK;
            }
        }
    }

    *symbol_id = symbol_pos;
    return PARSER_OK;
}

static inline uint8_t psr_scope_open(ScopeType scope_type)
{
    scope *new_scope = scope_crete(scope_type);
    if (new_scope == 0)
    {
        return SYMBOL_SCOPE_ERROR;
    }

    // 생성된 스코프의 심볼 개수 초기화
    new_scope->num = 0;
    new_scope->par = current_scope;
    current_scope = new_scope;
    return PARSER_OK;
}

static inline uint8_t psr_scope_close(void)
{
    if (current_scope == 0)
    {
        return SYMBOL_SCOPE_ERROR;
    }

    current_scope = scope_delate(current_scope);
    return PARSER_OK;
}

static inline void psr_scope_close_all(void)
{
    while (current_scope != 0)
    {
        psr_scope_close();
    }
}

#endif
