#include "parser.h"
#include "lexer.h"

check_P parser(ast_node *node_arr, token *token_arr, uint16_t token_number)
{

    /*
        파서 로직
        현재는 LL(1) 파서임
        ? 나중에 LR 파서로 바꿀 의항이 있음
    */

    // 파서 쪽에서 에러나면 확인하는 구조체
    // ! 이름 변경 필요
    check_P a;

    // 받은 배열을 전부 순회
    while (1)
    {
        /*
            지금 보고 있는 배열의 원소가 파싱표에 일치하는 지 확인
            그 후 추상노드를 리턴하는 파서 함수를 호출
        */

        if (1)
        {
            ;
        }
        else if (1)
        {
        }
    }
}

ast_node parser_stat(ast_node *node_arr, token *token_arr) { ; }
ast_node parser_decl(ast_node *node_arr, token *token_arr) { ; }
ast_node parser_expr(ast_node *node_arr, token *token_arr) { ; }
ast_node parser_block(ast_node *node_arr, token *token_arr) { ; }