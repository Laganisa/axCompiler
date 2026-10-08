#include "parser.h"
#include "token.h"

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
    // 자료형, 이름, 여는 괄호 순서면 함수 정의로 파싱한다
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

        // 괄호 확인
        if (psr_token_type_at(token_arr, *see_token, token_number) != SO_BRACKET_TOKEN)
        {
            return psr_result(NOT_SO_BRACKET, PSR_NO_NODE);
        }
        (*see_token)++;

        uint8_t scope_state = psr_scope_open(SCOPE_FUNCTION);
        if (scope_state != PARSER_OK)
        {
            return psr_result(scope_state, PSR_NO_NODE);
        }

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
                uint16_t param_symbol_id = 0;
                uint8_t param_symbol_state = psr_symbol_create(
                    token_arr,
                    param_name_pos,
                    &param_symbol_id);
                if (param_symbol_state != PARSER_OK)
                {
                    return psr_result(param_symbol_state, PSR_NO_NODE);
                }

                psr_init_node(&new_node, VAR_NODE);
                new_node.value.sym = param_symbol_id;
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
            token_number,
            TRUE);
        if (body_state.error_code != PARSER_OK)
        {
            return body_state;
        }

        scope_state = psr_scope_close();
        if (scope_state != PARSER_OK)
        {
            return psr_result(scope_state, PSR_NO_NODE);
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
    // 조건을 확인한 뒤 본문과 선택적인 else 문을 연결한다
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
            node_arr, token_arr, see_token, token_number, 1);
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
            token_number,
            FALSE);
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
    // while 조건과 본문을 반복 노드에 연결한다
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
            node_arr, token_arr, see_token, token_number, 1);
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
            token_number,
            FALSE);
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
    // for 초기식, 조건식, 반복식, 본문을 차례로 파싱한다
    if (current_type == FOR_TOKEN)
    {
        (*see_token)++;
        if (psr_token_type_at(token_arr, *see_token, token_number) != SO_BRACKET_TOKEN)
        {
            return psr_result(NOT_SO_BRACKET, PSR_NO_NODE);
        }
        (*see_token)++;

        uint8_t scope_state = psr_scope_open(SCOPE_BLOCK);
        if (scope_state != PARSER_OK)
        {
            return psr_result(scope_state, PSR_NO_NODE);
        }

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
                    node_arr, token_arr, see_token, token_number, 1);
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
                node_arr, token_arr, see_token, token_number, 1);
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
                node_arr, token_arr, see_token, token_number, 1);

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
            token_number,
            FALSE);

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
        now_psr_state = psr_store_node(node_arr, new_node);
        if (now_psr_state.error_code != PARSER_OK)
        {
            return now_psr_state;
        }

        scope_state = psr_scope_close();
        if (scope_state != PARSER_OK)
        {
            return psr_result(scope_state, PSR_NO_NODE);
        }
        return now_psr_state;
    }

    // 반환문 (return) 처리
    // 반환식이 있으면 반환 노드에 연결한다
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
                token_number,
                1);
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
            token_number,
            FALSE);
    }

    // 표현식 처리
    check_Parser now_psr_state = psr_expr(
        node_arr,
        token_arr,
        see_token,
        token_number,
        1);
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
