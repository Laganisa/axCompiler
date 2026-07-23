#include "parser.h"
#include "lexer.h"
#include "token.h"

check_P parser(ast_node *node_arr, token *token_arr, uint16_t token_number)
{

    /*
        파서 로직
        현재는 LL(1) 파서임
        ? 나중에 LR 파서로 바꿀 의항이 있음
    */

    // 파서 쪽에서 에러나면 확인하는 구조체
    check_P result;

    // 파서 스택
    parser_node_buf parser_stack;

    // 내가 보고 있는 토큰
    uint16_t see_token = 0;

    // 받은 배열을 전부 순회
    while (see_token < token_number)
    {
        /*
            지금 보고 있는 배열의 원소가 파싱표에 일치하는 지 확인
            그 후 추상노드를 리턴하는 파서 함수를 호출
        */

        // if 노드인지
        if (token_arr[see_token].type == IF_TOKEN)
        {
            ast_node now_ast_node;
        }
        else if (1)
        {
        }

        // 보고 있는 값을 증가?
        // ! 나중에 수정
        see_token++;
    }
}

// 재귀를 위한 함수
ast_node parser_stmt(ast_node *node_arr, token *token_arr) { ; }
ast_node parser_decl(ast_node *node_arr, token *token_arr) { ; }
ast_node parser_expr(ast_node *node_arr, token *token_arr) { ; }
ast_node parser_block(ast_node *node_arr, token *token_arr) { ; }