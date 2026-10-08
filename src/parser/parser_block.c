#include "parser.h"
#include "token.h"

/*
    블럭의 어휘 분석
*/
check_Parser psr_block(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number,
    uint8_t function_body)
{
    // 종료 조건
    if (node_arr == 0 || token_arr == 0 || see_token == 0)
    {
        return psr_result(UNEXPECTED_TOKEN, PSR_NO_NODE);
    }

    if (psr_token_type_at(token_arr, *see_token, token_number) != MO_BRACKET_TOKEN)
    {
        return psr_result(NOT_MO_BRACKET, PSR_NO_NODE);
    }
    (*see_token)++;

    uint8_t scope_state = PARSER_OK;
    // 함수 본문은 매개변수와 같은 스코프를 사용
    if (!function_body)
    {
        scope_state = psr_scope_open(SCOPE_BLOCK);
        if (scope_state != PARSER_OK)
        {
            return psr_result(scope_state, PSR_NO_NODE);
        }
    }

    // 블럭 안의 문장 목록 시작과 끝을 기억한다
    uint16_t head_pos = PSR_NO_NODE;
    uint16_t tail_pos = PSR_NO_NODE;

    // 중괄호가 나올 때 까지 파싱
    while (psr_token_type_at(token_arr, *see_token, token_number) != MC_BRACKET_TOKEN)
    {
        if (*see_token >= token_number ||
            psr_token_type_at(token_arr, *see_token, token_number) == END_TOKEN)
        {
            return psr_result(NOT_MC_BRACKET, PSR_NO_NODE);
        }

        uint16_t token_pos_before_stmt = *see_token;

        // 문장 파싱
        check_Parser now_psr_state = psr_stmt(
            node_arr,
            token_arr,
            see_token,
            token_number);
        if (now_psr_state.error_code != PARSER_OK)
        {
            // 재귀로 받은 노드 확인
            return now_psr_state;
        }
        if (*see_token == token_pos_before_stmt)
        {
            return psr_result(UNEXPECTED_TOKEN, PSR_NO_NODE);
        }

        // 생성된 문장을 목록 노드에 넣기
        ast_node new_node = {0};
        psr_init_node(&new_node, STMT_NODE);
        new_node.left = now_psr_state.pos;
        check_Parser list_state = psr_store_node(node_arr, new_node);
        if (list_state.error_code != PARSER_OK)
        {
            return list_state;
        }
        if (head_pos == PSR_NO_NODE)
        {
            head_pos = list_state.pos;
        }
        // stmt 노드를 다음 문장으로 연결
        else
        {
            node_arr[tail_pos].right = list_state.pos;
        }
        tail_pos = list_state.pos;
    }
    (*see_token)++;

    // 블럭 노드 생성
    // 문장 목록을 블럭 노드에 연결한다
    ast_node new_node = {0};
    psr_init_node(&new_node, BLOCK_NODE);
    new_node.left = head_pos;
    check_Parser block_state = psr_store_node(node_arr, new_node);
    if (block_state.error_code != PARSER_OK)
    {
        return block_state;
    }

    if (!function_body)
    {
        scope_state = psr_scope_close();
        if (scope_state != PARSER_OK)
        {
            return psr_result(scope_state, PSR_NO_NODE);
        }
    }
    return block_state;
}
