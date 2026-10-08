#include "parser.h"
#include "token.h"

/*
    ! 나중에 일일이 따라가며 수정하기 !
*/

// 재귀 중에 쌓이는 노드와 스코프 위치
uint16_t global_node_pos = 0; // 모든 함수에서 사용가능한 어떤 노드 위치에 넣을지
scope *current_scope = 0;

/*
    추상 노드에 넣기 위해 토큰 배열과 그 배열의 개수를 입력 받음
*/
check_Parser parser(
    ast_node *node_arr,
    token *token_arr,
    uint16_t token_number)
{
    /*
        파서 로직
        현재는 재귀 LL(1) 파서임
    */

    // 파서 쪽에서 에러나면 확인하는 구조체
    // 새 파싱을 시작할 때 이전 노드와 스코프를 비운다
    // 노드 위치 초기화
    global_node_pos = 0;
    psr_scope_close_all();

    // 입력 배열이 없으면 파싱을 시작하지 않는다
    if (node_arr == 0 || (token_arr == 0 && token_number != 0))
    {
        return psr_result(UNEXPECTED_TOKEN, PSR_NO_NODE);
    }

    // 최상위 문장들이 공유할 전역 스코프를 연다
    uint8_t scope_state = psr_scope_open(SCOPE_GLOBAL);

    if (scope_state != PARSER_OK)
    {
        return psr_result(scope_state, PSR_NO_NODE);
    }

    // 내가 보고 있는 토큰
    uint16_t see_token = 0;
    uint16_t head_pos = PSR_NO_NODE;
    uint16_t tail_pos = PSR_NO_NODE;

    // 받은 배열을 전부 순회
    // 입력 토큰을 문장 단위로 모두 파싱한다
    while (see_token < token_number &&
           psr_token_type_at(token_arr, see_token, token_number) != END_TOKEN)
    {
        uint16_t token_pos_before_stmt = see_token;

        // 문장 파싱
        check_Parser new_psr_state = psr_stmt(
            node_arr,
            token_arr,
            &see_token,
            token_number);

        if (new_psr_state.error_code != PARSER_OK)
        {
            psr_scope_close_all();
            // 에러 확인
            return new_psr_state;
        }

        if (see_token == token_pos_before_stmt)
        {
            psr_scope_close_all();
            return psr_result(UNEXPECTED_TOKEN, PSR_NO_NODE);
        }

        // 생성된 문장을 목록 노드에 넣기
        // 문장 노드를 목록에 이어 붙인다
        ast_node stmt_node = {0};
        psr_init_node(&stmt_node, STMT_NODE);
        stmt_node.left = new_psr_state.pos;
        check_Parser list_state = psr_store_node(node_arr, stmt_node);

        if (list_state.error_code != PARSER_OK)
        {
            psr_scope_close_all();
            return list_state;
        }
        // 파서 노드가 없다면
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

    // 최상위 문장 목록을 루트 블럭으로 연결
    // 문장 목록을 루트 블럭에 넣고 전역 스코프를 닫는다
    ast_node root_node = {0};
    psr_init_node(&root_node, BLOCK_NODE);
    root_node.left = head_pos;
    check_Parser root_state = psr_store_node(node_arr, root_node);
    psr_scope_close_all();
    return root_state;
}
