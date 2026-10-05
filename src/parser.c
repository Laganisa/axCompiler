#include "parser.h"
#include "lexer.h"
#include "token.h"
#include "symbol.h"

// 재귀를 위한 함수
static uint16_t psr_token_type_at(
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

uint16_t global_node_pos = 0; // 모든 함수에서 사용가능한 어떤 노드 위치에 넣을지

/*
    문장의 어휘 분석
*/
check_Parser psr_stmt(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number)
{
    // 리턴할 파서 상태
    check_Parser new_psr_state = {0};

    // 배열에 쓸 새로운 노드
    ast_node new_node = {0};

    // 재귀함수 호출에 쓰일 리턴 구조체
    check_Parser now_psr_state = {0};

    new_psr_state.error_code = UNEXPECTED_TOKEN;

    // 종료 조건
    if (node_arr == 0 || token_arr == 0 ||
        see_token == 0 || *see_token >= token_number)
    {
        return new_psr_state;
    }

    uint16_t current_type = psr_token_type_at(token_arr, *see_token, token_number);

    // 분기문의 어휘 분석
    if (current_type == IF_TOKEN)
    {
        // 분기 토큰 소비
        (*see_token)++;

        // 괄호 확인
        if (psr_token_type_at(token_arr, *see_token, token_number) != SO_BRACKET_TOKEN)
        {
            // 구문 오류
            new_psr_state.error_code = NOT_SO_BRACKET;
            return new_psr_state;
        }

        (*see_token)++;

        // 표현식 파싱
        // mid에 들어갈 변수
        now_psr_state = psr_expr(
            node_arr,
            token_arr,
            see_token,
            token_number);

        // 재귀로 받은 노드 확인
        if (now_psr_state.error_code != PARSER_OK)
        {
            new_psr_state.error_code = now_psr_state.error_code;
            return new_psr_state;
        }

        new_node.mid = now_psr_state.pos;

        // 괄호 확인
        if (psr_token_type_at(token_arr, *see_token, token_number) != SC_BRACKET_TOKEN)
        {
            // 구문 오류
            new_psr_state.error_code = NOT_SC_BRACKET;
            return new_psr_state;
        }

        (*see_token)++;

        // 블록 파싱
        // left에 들어갈거
        now_psr_state = psr_block(
            node_arr,
            token_arr,
            see_token,
            token_number);

        // 재귀로 받은 노드 확인
        if (now_psr_state.error_code != PARSER_OK)
        {
            new_psr_state.error_code = now_psr_state.error_code;
            return new_psr_state;
        }

        // left에 넣기
        new_node.left = now_psr_state.pos;

        // else 어휘 분석
        // right에 들어갈 거
        if (psr_token_type_at(token_arr, *see_token, token_number) == ELSE_TOKEN)
        {
            (*see_token)++;

            // 다음이 if 토큰이면?
            if (psr_token_type_at(token_arr, *see_token, token_number) == IF_TOKEN)
            {
                now_psr_state = psr_stmt(
                    node_arr,
                    token_arr,
                    see_token,
                    token_number);
            }
            // 아니면 블럭 호출
            else
            {
                now_psr_state = psr_block(
                    node_arr,
                    token_arr,
                    see_token,
                    token_number);
            }

            if (now_psr_state.error_code != PARSER_OK)
            {
                new_psr_state.error_code = now_psr_state.error_code;
                return new_psr_state;
            }

            new_node.right = now_psr_state.pos;
        }

        // 마무리

        new_node.node_type = IF_NODE;
    }

    // 반복문의 어휘 분석
    // 조건 반복문 및 한정 반복문
    else if (current_type == WHILE_TOKEN || current_type == FOR_TOKEN)
    {
        (*see_token)++;

        // 괄호 확인
        if (psr_token_type_at(token_arr, *see_token, token_number) != SO_BRACKET_TOKEN)
        {
            // 구문 오류
            new_psr_state.error_code = NOT_SO_BRACKET;
            return new_psr_state;
        }

        (*see_token)++;

        // for 문이라면?
        if (current_type == FOR_TOKEN)
        {
            // 파싱
            // 선언일 경우
            if (
                psr_token_type_at(token_arr, *see_token, token_number) >= UINT8_TOKEN &&
                psr_token_type_at(token_arr, *see_token, token_number) <= INT64_TOKEN)
            {
                now_psr_state = psr_decl(
                    node_arr,
                    token_arr,
                    see_token,
                    token_number);
            }
            // 대입일 경우
            else
            {
                now_psr_state = psr_expr(
                    node_arr,
                    token_arr,
                    see_token,
                    token_number);
            }

            if (now_psr_state.error_code != PARSER_OK)
            {
                new_psr_state.error_code = now_psr_state.error_code;
                return new_psr_state;
            }

            new_node.left = now_psr_state.pos;
        }

        // 조건식 파싱
        now_psr_state = psr_expr(
            node_arr,
            token_arr,
            see_token,
            token_number);

        // 재귀로 받은 노드 확인
        if (now_psr_state.error_code != PARSER_OK)
        {
            new_psr_state.error_code = now_psr_state.error_code;
            return new_psr_state;
        }

        new_node.mid = now_psr_state.pos;

        // 괄호 확인
        if (psr_token_type_at(token_arr, *see_token, token_number) != SC_BRACKET_TOKEN)
        {
            // 구문 오류
            new_psr_state.error_code = NOT_SC_BRACKET;
            return new_psr_state;
        }

        (*see_token)++;

        // 오른쪽 파싱
        now_psr_state = psr_block(
            node_arr,
            token_arr,
            see_token,
            token_number);

        // 재귀로 받은 노드 확인
        if (now_psr_state.error_code != PARSER_OK)
        {
            new_psr_state.error_code = now_psr_state.error_code;
            return new_psr_state;
        }

        new_node.right = now_psr_state.pos;

        new_node.node_type = LOOP_NODE;
    }

    // 반환문 (return) 처리
    else if (current_type == RETURN_TOKEN)
    {
        (*see_token)++;

        now_psr_state = psr_expr(
            node_arr,
            token_arr,
            see_token,
            token_number);

        // 재귀로 받은 노드 확인
        if (now_psr_state.error_code != PARSER_OK)
        {
            new_psr_state.error_code = now_psr_state.error_code;
            return new_psr_state;
        }

        new_node.left = now_psr_state.pos;

        // 새미콜론이 없다면?
        if (psr_token_type_at(token_arr, *see_token, token_number) != SEMI_TOKEN)
        {
            // 구문 에러
            new_psr_state.error_code = NOT_SEMI;
            return new_psr_state;
        }

        (*see_token)++;

        // 마무리
        new_node.node_type = RETURN_NODE;
    }

    // 자료형으로 시작하는 선언문 처리 (int, uint 등)
    else if (current_type >= UINT8_TOKEN && current_type <= INT64_TOKEN)
    {
        now_psr_state = psr_decl(
            node_arr,
            token_arr,
            see_token,
            token_number);

        // 재귀로 받은 노드 확인
        if (now_psr_state.error_code != PARSER_OK)
        {
            new_psr_state.error_code = now_psr_state.error_code;
            return new_psr_state;
        }

        new_node.left = now_psr_state.pos;

        // 마무리
        new_node.node_type = DECL_NODE;
    }

    // 중괄호 블록 처리
    else if (current_type == MO_BRACKET_TOKEN)
    {
        now_psr_state = psr_block(
            node_arr,
            token_arr,
            see_token,
            token_number);

        // 재귀로 받은 노드 확인
        if (now_psr_state.error_code != PARSER_OK)
        {
            new_psr_state.error_code = now_psr_state.error_code;
            return new_psr_state;
        }

        new_node.left = now_psr_state.pos;
        new_node.node_type = BLOCK_NODE;
    }

    // 표현식 처리
    else if (current_type == VAL_TOKEN)
    {
        now_psr_state = psr_expr(
            node_arr,
            token_arr,
            see_token,
            token_number);

        // 재귀로 받은 노드 확인
        if (now_psr_state.error_code != PARSER_OK)
        {
            new_psr_state.error_code = now_psr_state.error_code;
            return new_psr_state;
        }

        new_node.left = now_psr_state.pos;
        new_node.node_type = STMT_NODE;
    }

    else
    {
        new_psr_state.error_code = UNEXPECTED_TOKEN;
        return new_psr_state;
    }

    // 생성된 new_node를 배열에 넣기
    // TODO(parser): 호출 간 공유되는 노드 할당기가 정해져야 AST가 완성됨.
    uint8_t npos = 0;

    node_arr[npos] = new_node;

    // 값을 넣고
    new_psr_state.node_pos++;
    new_psr_state.pos = npos;

    new_psr_state.error_code = PARSER_OK;
    return new_psr_state;
}

/*
    선언의 어휘 분석
*/
check_Parser psr_decl(ast_node *node_arr, token *token_arr, uint16_t *see_token, uint16_t token_number)
{
    check_Parser new_psr_state = {0};

    ast_node new_node = {0};

    // 재귀함수 호출에 쓰일 리턴 구조체
    check_Parser now_psr_state = {0};

    new_psr_state.error_code = UNEXPECTED_TOKEN;

    // 종료 조건
    if (node_arr == 0 || token_arr == 0 ||
        see_token == 0 || *see_token >= token_number)
    {
        return new_psr_state;
    }

    uint16_t current_type = psr_token_type_at(token_arr, *see_token, token_number);

    // 자료형 확인
    if (
        *see_token < token_number &&
        (current_type < UINT8_TOKEN ||
         current_type > INT64_TOKEN))
    {
        // 에러 처리
        new_psr_state.error_code = NOT_DATA_TYPE;
        return new_psr_state;
    }

    (*see_token)++;

    // 변수 이름인지 확인
    if (psr_token_type_at(token_arr, *see_token, token_number) != VAL_TOKEN)
    {
        new_psr_state.error_code = NOT_VAR_NAME;
        return new_psr_state;
    }

    (*see_token)++;

    // 변수명 넣기
    char *name = token_arr[*see_token].token_value_data.token_name;
    uint16_t name_id = symbol(name);

    new_node.value.sym = name_id;

    // 대입 확인하기
    if (psr_token_type_at(token_arr, *see_token, token_number) == ASSIGN_TOKEN)
    {
        (*see_token)++;

        now_psr_state = psr_expr(
            node_arr,
            token_arr,
            see_token,
            token_number);

        if (now_psr_state.error_code != PARSER_OK)
        {
            new_psr_state.error_code = now_psr_state.error_code;
            return new_psr_state;
        }

        new_node.right = now_psr_state.pos;
    }

    // 세미콜론
    if (psr_token_type_at(token_arr, *see_token, token_number) != SEMI_TOKEN)
    {
        new_psr_state.error_code = NOT_SEMI;
        return new_psr_state;
    }

    (*see_token)++;

    // 생성된 new_node를 배열에 넣기
    // TODO(parser): 호출 간 공유되는 노드 할당기가 정해져야 AST가 완성됨.
    uint8_t npos = 0;

    node_arr[npos] = new_node;

    // 값을 넣고
    new_psr_state.node_pos++;
    new_psr_state.pos = npos;

    new_psr_state.error_code = PARSER_OK;
    return new_psr_state;
}

/*
    표현식의 어휘 분석
*/
check_Parser psr_expr(ast_node *node_arr, token *token_arr, uint16_t *see_token, uint16_t token_number)
{
    check_Parser new_psr_state = {0};

    ast_node new_node = {0};

    // 재귀함수 호출에 쓰일 리턴 구조체
    check_Parser now_psr_state = {0};

    new_psr_state.error_code = UNEXPECTED_TOKEN;

    // while에서 쓸 변수
    uint8_t is_psr = TRUE;

    if (node_arr == 0 || token_arr == 0 ||
        see_token == 0 || *see_token >= token_number)
    {
        return new_psr_state;
    }

    while (*see_token < token_number)
    {
        uint16_t op_type = psr_token_type_at(token_arr, *see_token, token_number);

        // 연산자가 아니라면 리턴
        if (
            op_type < PLUS_TOKEN ||
            op_type > LOGICAL_OR_TOKEN)
        {
            is_psr = FALSE;
            break;
        }

        // 연산자 토큰 소비
        (*see_token)++;

        // 괄호가 있다면
        if (op_type == SO_BRACKET_TOKEN)
        {
            (*see_token)++;

            now_psr_state = psr_expr(
                node_arr,
                token_arr,
                see_token,
                token_number);

            if (now_psr_state.error_code != PARSER_OK)
            {
                new_psr_state.error_code = now_psr_state.error_code;
                return new_psr_state;
            }

            if (psr_token_type_at(token_arr, *see_token, token_number) != SC_BRACKET_TOKEN)
            {
                // 구문 오류
                new_psr_state.error_code = NOT_SC_BRACKET;
                return new_psr_state;
            }

            (*see_token)++;
        }

        // TODO(parser): 이항 연산자 우선순위와 피연산자 연결은 아직 의사 코드 단계임.
    }

    // 파싱이 참이 아니라면
    if (!is_psr)
    {
        new_psr_state.error_code = NOT_VAR_NAME;
        return new_psr_state;
    }

    uint8_t npos = 0;

    node_arr[npos] = new_node;

    // 값을 넣고
    new_psr_state.node_pos++;
    new_psr_state.pos = npos;

    new_psr_state.error_code = PARSER_OK;
    return new_psr_state;
}

/*
    블럭의 어휘 분석
*/
check_Parser psr_block(ast_node *node_arr, token *token_arr, uint16_t *see_token, uint16_t token_number)
{
    check_Parser new_psr_state = {0};

    check_Parser now_psr_state = {0};
    uint8_t has__closed_block = FALSE;

    new_psr_state.error_code = UNEXPECTED_TOKEN;

    if (node_arr == 0 || token_arr == 0 ||
        see_token == 0 || *see_token >= token_number)
    {
        return new_psr_state;
    }

    if (psr_token_type_at(token_arr, *see_token, token_number) != MO_BRACKET_TOKEN)
    {
        new_psr_state.error_code = NOT_MO_BRACKET;
        return new_psr_state;
    }

    (*see_token)++;

    // 중괄호가 나올 때 까지 파싱
    while (*see_token < token_number)
    {
        if (psr_token_type_at(token_arr, *see_token, token_number) == MC_BRACKET_TOKEN)
        {
            (*see_token)++;
            has__closed_block = TRUE;
            break;
        }

        uint16_t token__pos_before_stmt = *see_token;
        now_psr_state = psr_stmt(
            node_arr,
            token_arr,
            see_token,
            token_number);

        if (now_psr_state.error_code != PARSER_OK)
        {
            new_psr_state.error_code = now_psr_state.error_code;
            return new_psr_state;
        }

        if (*see_token == token__pos_before_stmt)
        {
            new_psr_state.error_code = UNEXPECTED_TOKEN;
            return new_psr_state;
        }

        // TODO(parser): 문장 목록의 AST 연결 방식은 아직 정해지지 않았음.
    }

    if (!has__closed_block)
    {
        new_psr_state.error_code = NOT_MC_BRACKET;
        return new_psr_state;
    }

    new_psr_state.error_code = PARSER_OK;
    return new_psr_state;
}

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
    check_Parser new_psr_state = {0};

    // 내가 보고 있는 토큰
    uint16_t see_token = 0;

    new_psr_state.error_code = PARSER_OK;

    if (node_arr == 0 || token_arr == 0)
    {
        new_psr_state.error_code = UNEXPECTED_TOKEN;
        return new_psr_state;
    }

    // 받은 배열을 전부 순회
    while (see_token < token_number)
    {
        uint16_t token__pos_before_stmt = see_token;
        new_psr_state = psr_stmt(
            node_arr,
            token_arr,
            &see_token,
            token_number);

        // 에러 확인
        if (new_psr_state.error_code != PARSER_OK)
        {
            break;
        }

        if (see_token == token__pos_before_stmt)
        {
            new_psr_state.error_code = UNEXPECTED_TOKEN;
            break;
        }

        // TODO(parser): 최상위 문장 목록의 AST 연결 방식은 아직 정해지지 않았음.
    }

    return new_psr_state;
}