#include "parser.h"
#include "token.h"

/*
    선언의 어휘 분석
*/
check_Parser psr_decl(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number)
{
    // 종료 조건
    if (node_arr == 0 || token_arr == 0 || see_token == 0)
    {
        return psr_result(UNEXPECTED_TOKEN, PSR_NO_NODE);
    }

    // 자료형 확인
    // 자료형과 변수 이름을 차례로 확인한다
    uint16_t current_type = psr_token_type_at(token_arr, *see_token, token_number);
    if (current_type < UINT8_TOKEN || current_type > INT64_TOKEN)
    {
        // 에러 처리
        return psr_result(NOT_DATA_TYPE, PSR_NO_NODE);
    }

    // 자료형 노드 생성
    ast_node new_node = {0};
    psr_init_node(&new_node, TYPE_NODE);
    new_node.value.type = (uint8_t)current_type;
    check_Parser type_state = psr_store_node(node_arr, new_node);

    if (type_state.error_code != PARSER_OK)
    {
        return type_state;
    }
    (*see_token)++;

    // 변수 이름인지 확인
    if (psr_token_type_at(token_arr, *see_token, token_number) != VAL_TOKEN)
    {
        // 변수 이름이 아님
        return psr_result(NOT_VAR_NAME, PSR_NO_NODE);
    }

    // 변수명 넣기
    uint16_t name_pos = *see_token;

    uint16_t symbol_id = 0;
    uint8_t symbol_state = psr_symbol_create(
        token_arr,
        name_pos,
        &symbol_id);

    if (symbol_state != PARSER_OK)
    {
        return psr_result(symbol_state, PSR_NO_NODE);
    }

    psr_init_node(&new_node, VAR_NODE);
    new_node.value.sym = symbol_id;
    check_Parser name_state = psr_store_node(node_arr, new_node);

    if (name_state.error_code != PARSER_OK)
    {
        return name_state;
    }
    (*see_token)++;

    psr_init_node(&new_node, DECL_NODE);
    new_node.left = type_state.pos;
    new_node.mid = name_state.pos;

    // 대입 확인하기
    // 초기값이 있으면 표현식 노드를 선언에 연결한다
    if (psr_token_type_at(token_arr, *see_token, token_number) == ASSIGN_TOKEN)
    {
        (*see_token)++;
        check_Parser expression_state = psr_expr(
            node_arr,
            token_arr,
            see_token,
            token_number,
            1);
        if (expression_state.error_code != PARSER_OK)
        {
            // 재귀로 받은 노드 확인
            return expression_state;
        }
        new_node.right = expression_state.pos;
    }

    // 세미콜론
    // 새미콜론이 없다면?
    if (psr_token_type_at(token_arr, *see_token, token_number) != SEMI_TOKEN)
    {
        // 구문 에러
        return psr_result(NOT_SEMI, PSR_NO_NODE);
    }
    (*see_token)++;

    return psr_store_node(node_arr, new_node);
}
