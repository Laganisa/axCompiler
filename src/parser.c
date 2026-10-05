#include "parser.h"
#include "token.h"

/*
    ! 나중에 일일이 따라가며 수정하기 !
*/

// 재귀를 위한 함수
uint16_t global_node_pos = 0; // 모든 함수에서 사용가능한 어떤 노드 위치에 넣을지

/*
    표현식의 어휘 분석
*/
check_Parser psr_expr(
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

    return psr_expr_prec(
        node_arr,
        token_arr,
        see_token,
        token_number,
        1);
}

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
    psr_init_node(&new_node, VAR_NODE);
    new_node.value.sym = psr_symbol_id(token_arr, name_pos);
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
    if (psr_token_type_at(token_arr, *see_token, token_number) == ASSIGN_TOKEN)
    {
        (*see_token)++;
        check_Parser expression_state = psr_expr(
            node_arr,
            token_arr,
            see_token,
            token_number);
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

/*
    블럭의 어휘 분석
*/
check_Parser psr_block(
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

    if (psr_token_type_at(token_arr, *see_token, token_number) != MO_BRACKET_TOKEN)
    {
        return psr_result(NOT_MO_BRACKET, PSR_NO_NODE);
    }
    (*see_token)++;

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
    ast_node new_node = {0};
    psr_init_node(&new_node, BLOCK_NODE);
    new_node.left = head_pos;
    return psr_store_node(node_arr, new_node);
}

/*
    문장의 어휘 분석
*/
check_Parser psr_stmt(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number)
{
    // 종료 조건
    if (node_arr == 0 || token_arr == 0 || see_token == 0 ||
        *see_token >= token_number)
    {
        return psr_result(UNEXPECTED_TOKEN, PSR_NO_NODE);
    }

    uint16_t current_type = psr_token_type_at(token_arr, *see_token, token_number);

    // 함수의 어휘 분석
    if (current_type >= UINT8_TOKEN &&
        current_type <= INT64_TOKEN &&
        token_number - *see_token > 2 &&
        psr_token_type_at(token_arr, *see_token + 1, token_number) == VAL_TOKEN &&
        psr_token_type_at(token_arr, *see_token + 2, token_number) == SO_BRACKET_TOKEN)
    {
        // 자료형 확인
        ast_node new_node = {0};
        psr_init_node(&new_node, TYPE_NODE);
        new_node.value.type = (uint8_t)current_type;
        check_Parser type_state = psr_store_node(node_arr, new_node);
        if (type_state.error_code != PARSER_OK)
        {
            return type_state;
        }
        (*see_token)++;

        // 함수 이름인지 확인
        uint16_t name_pos = *see_token;
        psr_init_node(&new_node, VAR_NODE);
        new_node.value.sym = psr_symbol_id(token_arr, name_pos);
        check_Parser name_state = psr_store_node(node_arr, new_node);
        if (name_state.error_code != PARSER_OK)
        {
            return name_state;
        }
        (*see_token)++;

        // 괄호 확인
        if (psr_token_type_at(token_arr, *see_token, token_number) != SO_BRACKET_TOKEN)
        {
            return psr_result(NOT_SO_BRACKET, PSR_NO_NODE);
        }
        (*see_token)++;

        uint16_t param_head = PSR_NO_NODE;
        uint16_t param_tail = PSR_NO_NODE;

        // 매개변수가 나올 때 까지 파싱
        if (psr_token_type_at(token_arr, *see_token, token_number) != SC_BRACKET_TOKEN)
        {
            while (TRUE)
            {
                // 자료형 확인
                uint16_t param_type = psr_token_type_at(token_arr, *see_token, token_number);
                if (param_type < UINT8_TOKEN || param_type > INT64_TOKEN)
                {
                    return psr_result(NOT_DATA_TYPE, PSR_NO_NODE);
                }

                psr_init_node(&new_node, TYPE_NODE);
                new_node.value.type = (uint8_t)param_type;
                check_Parser param_type_state = psr_store_node(node_arr, new_node);
                if (param_type_state.error_code != PARSER_OK)
                {
                    return param_type_state;
                }
                (*see_token)++;

                // 변수 이름인지 확인
                if (psr_token_type_at(token_arr, *see_token, token_number) != VAL_TOKEN)
                {
                    return psr_result(NOT_VAR_NAME, PSR_NO_NODE);
                }

                uint16_t param_name_pos = *see_token;
                psr_init_node(&new_node, VAR_NODE);
                new_node.value.sym = psr_symbol_id(token_arr, param_name_pos);
                check_Parser param_name_state = psr_store_node(node_arr, new_node);
                if (param_name_state.error_code != PARSER_OK)
                {
                    return param_name_state;
                }
                (*see_token)++;

                // 매개변수 노드 생성
                psr_init_node(&new_node, PARAM_NODE);
                new_node.left = param_type_state.pos;
                new_node.mid = param_name_state.pos;
                check_Parser param_state = psr_store_node(node_arr, new_node);
                if (param_state.error_code != PARSER_OK)
                {
                    return param_state;
                }

                if (param_head == PSR_NO_NODE)
                {
                    param_head = param_state.pos;
                }
                else
                {
                    node_arr[param_tail].right = param_state.pos;
                }
                param_tail = param_state.pos;

                if (psr_token_type_at(token_arr, *see_token, token_number) != COMMA_TOKEN)
                {
                    break;
                }
                (*see_token)++;
            }
        }

        // 닫는 괄호 확인
        if (psr_token_type_at(token_arr, *see_token, token_number) != SC_BRACKET_TOKEN)
        {
            return psr_result(NOT_SC_BRACKET, PSR_NO_NODE);
        }
        (*see_token)++;

        // 함수 본문 파싱
        check_Parser body_state = psr_block(
            node_arr,
            token_arr,
            see_token,
            token_number);
        if (body_state.error_code != PARSER_OK)
        {
            return body_state;
        }

        // 함수 노드 생성
        // left=반환 자료형, mid=매개변수 목록, right=본문, sym=함수 이름
        psr_init_node(&new_node, FUNCTION_NODE);
        new_node.value.sym = node_arr[name_state.pos].value.sym;
        new_node.left = type_state.pos;
        new_node.mid = param_head;
        new_node.right = body_state.pos;
        return psr_store_node(node_arr, new_node);
    }

    // 분기문의 어휘 분석
    if (current_type == IF_TOKEN)
    {
        // 분기 토큰 소비
        (*see_token)++;

        // 괄호 확인
        if (psr_token_type_at(token_arr, *see_token, token_number) != SO_BRACKET_TOKEN)
        {
            // 구문 오류
            return psr_result(NOT_SO_BRACKET, PSR_NO_NODE);
        }
        (*see_token)++;

        // 표현식 파싱
        // mid에 들어갈 변수
        check_Parser now_psr_state = psr_expr(
            node_arr, token_arr, see_token, token_number);
        if (now_psr_state.error_code != PARSER_OK)
        {
            // 재귀로 받은 노드 확인
            return now_psr_state;
        }
        uint16_t condition_pos = now_psr_state.pos;

        // 괄호 확인
        if (psr_token_type_at(token_arr, *see_token, token_number) != SC_BRACKET_TOKEN)
        {
            // 구문 오류
            return psr_result(NOT_SC_BRACKET, PSR_NO_NODE);
        }
        (*see_token)++;

        // 블록 파싱
        // left에 들어갈거
        now_psr_state = psr_block(
            node_arr,
            token_arr,
            see_token,
            token_number);
        if (now_psr_state.error_code != PARSER_OK)
        {
            // 재귀로 받은 노드 확인
            return now_psr_state;
        }
        uint16_t body_pos = now_psr_state.pos;
        uint16_t else_pos = PSR_NO_NODE;

        // else 어휘 분석
        // right에 들어갈 거
        if (psr_token_type_at(token_arr, *see_token, token_number) == ELSE_TOKEN)
        {
            (*see_token)++;

            // 다음이 if 토큰이면?
            // 아니면 블럭 호출
            now_psr_state = psr_stmt(
                node_arr,
                token_arr,
                see_token,
                token_number);
            if (now_psr_state.error_code != PARSER_OK)
            {
                return now_psr_state;
            }
            else_pos = now_psr_state.pos;
        }

        ast_node new_node = {0};
        psr_init_node(&new_node, IF_NODE);
        // left에 넣기
        new_node.left = body_pos;
        new_node.mid = condition_pos;
        new_node.right = else_pos;
        return psr_store_node(node_arr, new_node);
    }

    // 반복문의 어휘 분석
    // 조건 반복문 및 한정 반복문
    if (current_type == WHILE_TOKEN)
    {
        (*see_token)++;

        // 괄호 확인
        if (psr_token_type_at(token_arr, *see_token, token_number) != SO_BRACKET_TOKEN)
        {
            // 구문 오류
            return psr_result(NOT_SO_BRACKET, PSR_NO_NODE);
        }
        (*see_token)++;

        // 조건식 파싱
        check_Parser now_psr_state = psr_expr(
            node_arr, token_arr, see_token, token_number);
        if (now_psr_state.error_code != PARSER_OK)
        {
            // 재귀로 받은 노드 확인
            return now_psr_state;
        }
        uint16_t condition_pos = now_psr_state.pos;
        if (psr_token_type_at(token_arr, *see_token, token_number) != SC_BRACKET_TOKEN)
        {
            return psr_result(NOT_SC_BRACKET, PSR_NO_NODE);
        }
        (*see_token)++;

        // 오른쪽 파싱
        now_psr_state = psr_block(
            node_arr,
            token_arr,
            see_token,
            token_number);
        if (now_psr_state.error_code != PARSER_OK)
        {
            // 재귀로 받은 노드 확인
            return now_psr_state;
        }

        ast_node new_node = {0};
        psr_init_node(&new_node, LOOP_NODE);
        new_node.value.op = WHILE_TOKEN;
        new_node.mid = condition_pos;
        // 마무리
        new_node.right = now_psr_state.pos;
        return psr_store_node(node_arr, new_node);
    }

    // for 문이라면?
    if (current_type == FOR_TOKEN)
    {
        (*see_token)++;
        if (psr_token_type_at(token_arr, *see_token, token_number) != SO_BRACKET_TOKEN)
        {
            return psr_result(NOT_SO_BRACKET, PSR_NO_NODE);
        }
        (*see_token)++;

        uint16_t init_pos = PSR_NO_NODE;
        uint16_t condition_pos = PSR_NO_NODE;
        uint16_t increment_pos = PSR_NO_NODE;
        check_Parser now_psr_state = psr_result(PARSER_OK, PSR_NO_NODE);

        if (psr_token_type_at(token_arr, *see_token, token_number) != SEMI_TOKEN)
        {
            // 파싱
            // 선언일 경우
            if (psr_token_type_at(token_arr, *see_token, token_number) >= UINT8_TOKEN &&
                psr_token_type_at(token_arr, *see_token, token_number) <= INT64_TOKEN)
            {
                now_psr_state = psr_decl(
                    node_arr,
                    token_arr,
                    see_token,
                    token_number);
            }
            else
            {
                // 대입일 경우
                now_psr_state = psr_expr(
                    node_arr, token_arr, see_token, token_number);
                if (now_psr_state.error_code == PARSER_OK)
                {
                    ast_node init_node = {0};
                    psr_init_node(&init_node, STMT_NODE);
                    init_node.left = now_psr_state.pos;
                    now_psr_state = psr_store_node(node_arr, init_node);
                    if (now_psr_state.error_code == PARSER_OK &&
                        psr_token_type_at(token_arr, *see_token, token_number) == SEMI_TOKEN)
                    {
                        (*see_token)++;
                    }
                    else if (now_psr_state.error_code == PARSER_OK)
                    {
                        return psr_result(NOT_SEMI, PSR_NO_NODE);
                    }
                }
            }
            if (now_psr_state.error_code != PARSER_OK)
            {
                // 재귀로 받은 노드 확인
                return now_psr_state;
            }
            init_pos = now_psr_state.pos;
        }
        else
        {
            (*see_token)++;
        }

        // 조건식 파싱
        if (psr_token_type_at(token_arr, *see_token, token_number) != SEMI_TOKEN)
        {
            now_psr_state = psr_expr(
                node_arr, token_arr, see_token, token_number);
            if (now_psr_state.error_code != PARSER_OK)
            {
                // 재귀로 받은 노드 확인
                return now_psr_state;
            }
            condition_pos = now_psr_state.pos;
        }
        if (psr_token_type_at(token_arr, *see_token, token_number) != SEMI_TOKEN)
        {
            return psr_result(NOT_SEMI, PSR_NO_NODE);
        }
        (*see_token)++;

        // 괄호 확인
        if (psr_token_type_at(token_arr, *see_token, token_number) != SC_BRACKET_TOKEN)
        {
            now_psr_state = psr_expr(
                node_arr, token_arr, see_token, token_number);

            if (now_psr_state.error_code != PARSER_OK)
            {
                return now_psr_state;
            }

            increment_pos = now_psr_state.pos;
        }

        if (psr_token_type_at(token_arr, *see_token, token_number) != SC_BRACKET_TOKEN)
        {
            return psr_result(NOT_SC_BRACKET, PSR_NO_NODE);
        }

        (*see_token)++;

        // 오른쪽 파싱
        now_psr_state = psr_block(
            node_arr,
            token_arr,
            see_token,
            token_number);

        if (now_psr_state.error_code != PARSER_OK)
        {
            return now_psr_state;
        }
        uint16_t body_pos = now_psr_state.pos;

        // for의 반복식과 본문은 보조 문장 노드의 left/right에 보관한다.
        ast_node step_node = {0};
        psr_init_node(&step_node, STMT_NODE);
        step_node.left = increment_pos;
        step_node.right = body_pos;
        now_psr_state = psr_store_node(node_arr, step_node);
        if (now_psr_state.error_code != PARSER_OK)
        {
            return now_psr_state;
        }

        // 생성된 반복 노드에 반복식과 본문을 연결
        ast_node new_node = {0};
        psr_init_node(&new_node, LOOP_NODE);
        new_node.value.op = FOR_TOKEN;
        new_node.left = init_pos;
        new_node.mid = condition_pos;
        // 마무리
        new_node.right = now_psr_state.pos;
        return psr_store_node(node_arr, new_node);
    }

    // 반환문 (return) 처리
    if (current_type == RETURN_TOKEN)
    {
        (*see_token)++;
        // 반환 노드 생성
        ast_node new_node = {0};
        psr_init_node(&new_node, RETURN_NODE);
        if (psr_token_type_at(token_arr, *see_token, token_number) != SEMI_TOKEN)
        {
            // 표현식 파싱
            check_Parser now_psr_state = psr_expr(
                node_arr,
                token_arr,
                see_token,
                token_number);
            if (now_psr_state.error_code != PARSER_OK)
            {
                return now_psr_state;
            }
            new_node.left = now_psr_state.pos;
        }
        if (psr_token_type_at(token_arr, *see_token, token_number) != SEMI_TOKEN)
        {
            return psr_result(NOT_SEMI, PSR_NO_NODE);
        }
        (*see_token)++;
        return psr_store_node(node_arr, new_node);
    }

    // 자료형으로 시작하는 선언문 처리 (int, uint 등)
    if (current_type >= UINT8_TOKEN && current_type <= INT64_TOKEN)
    {
        return psr_decl(
            node_arr,
            token_arr,
            see_token,
            token_number);
    }

    // 중괄호 블록 처리
    if (current_type == MO_BRACKET_TOKEN)
    {
        return psr_block(
            node_arr,
            token_arr,
            see_token,
            token_number);
    }

    // 표현식 처리
    check_Parser now_psr_state = psr_expr(
        node_arr,
        token_arr,
        see_token,
        token_number);
    if (now_psr_state.error_code != PARSER_OK)
    {
        return now_psr_state;
    }
    if (psr_token_type_at(token_arr, *see_token, token_number) != SEMI_TOKEN)
    {
        return psr_result(NOT_SEMI, PSR_NO_NODE);
    }
    (*see_token)++;

    // 문장 노드 생성
    ast_node new_node = {0};
    psr_init_node(&new_node, STMT_NODE);
    new_node.left = now_psr_state.pos;
    return psr_store_node(node_arr, new_node);
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
    // 노드 위치 초기화
    global_node_pos = 0;
    if (node_arr == 0 || (token_arr == 0 && token_number != 0))
    {
        return psr_result(UNEXPECTED_TOKEN, PSR_NO_NODE);
    }

    // 내가 보고 있는 토큰
    uint16_t see_token = 0;
    uint16_t head_pos = PSR_NO_NODE;
    uint16_t tail_pos = PSR_NO_NODE;

    // 받은 배열을 전부 순회
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
            // 에러 확인
            return new_psr_state;
        }

        if (see_token == token_pos_before_stmt)
        {
            return psr_result(UNEXPECTED_TOKEN, PSR_NO_NODE);
        }

        // 생성된 문장을 목록 노드에 넣기
        ast_node stmt_node = {0};
        psr_init_node(&stmt_node, STMT_NODE);
        stmt_node.left = new_psr_state.pos;
        check_Parser list_state = psr_store_node(node_arr, stmt_node);

        if (list_state.error_code != PARSER_OK)
        {
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
    ast_node root_node = {0};
    psr_init_node(&root_node, BLOCK_NODE);
    root_node.left = head_pos;
    return psr_store_node(node_arr, root_node);
}
