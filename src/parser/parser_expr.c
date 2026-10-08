#include "parser.h"
#include "token.h"

/*
    표현식의 어휘 분석
*/
check_Parser psr_expr(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number,
    uint8_t min_precedence)
{
    // 종료 조건
    if (node_arr == 0 || token_arr == 0 || see_token == 0)
    {
        return psr_result(UNEXPECTED_TOKEN, PSR_NO_NODE);
    }

    // 왼쪽 피연산자 파싱에 필요한 값
    ast_node new_node = {0};
    uint16_t current_type = psr_token_type_at(token_arr, *see_token, token_number);
    // 재귀함수 호출에 쓰일 리턴 구조체
    check_Parser new_psr_state = psr_result(UNEXPECTED_TOKEN, PSR_NO_NODE);

    // 단항 연산자 처리
    if (current_type == PLUS_TOKEN ||
        current_type == MINU_TOKEN ||
        current_type == NOT_TOKEN ||
        current_type == PTR_TOKEN)
    {
        uint16_t operator_type = current_type;
        (*see_token)++;

        check_Parser operand_state = psr_expr(
            node_arr,
            token_arr,
            see_token,
            token_number,
            11);
        if (operand_state.error_code != PARSER_OK)
        {
            return operand_state;
        }

        psr_init_node(&new_node, operator_type == PTR_TOKEN ? PTR_NODE : UNARY_OP_NODE);
        new_node.value.op = (uint8_t)operator_type;
        new_node.right = operand_state.pos;
        new_psr_state = psr_store_node(node_arr, new_node);
    }
    // 괄호가 있다면
    else if (current_type == SO_BRACKET_TOKEN)
    {
        (*see_token)++;

        new_psr_state = psr_expr(
            node_arr,
            token_arr,
            see_token,
            token_number,
            1);
        if (new_psr_state.error_code != PARSER_OK)
        {
            return new_psr_state;
        }

        if (psr_token_type_at(token_arr, *see_token, token_number) != SC_BRACKET_TOKEN)
        {
            return psr_result(NOT_SC_BRACKET, PSR_NO_NODE);
        }
        (*see_token)++;
    }
    // 변수 이름 처리
    else if (current_type == VAL_TOKEN)
    {
        uint16_t name_pos = *see_token;
        uint16_t symbol_id = 0;
        uint8_t symbol_state = psr_symbol_search(
            token_arr,
            name_pos,
            &symbol_id);
        if (symbol_state != PARSER_OK)
        {
            return psr_result(symbol_state, PSR_NO_NODE);
        }

        psr_init_node(&new_node, VAR_NODE);
        new_node.value.sym = symbol_id;
        (*see_token)++;
        new_psr_state = psr_store_node(node_arr, new_node);
    }
    // 숫자 처리
    else if (current_type == FLOAT_NUMBER_TOKEN)
    {
        psr_init_node(&new_node, FLOAT_NODE);
        new_node.value.sym = *see_token;
        (*see_token)++;
        new_psr_state = psr_store_node(node_arr, new_node);
    }
    else if (current_type == BIN_NUMBER_TOKEN ||
             current_type == DEC_NUMBER_TOKEN ||
             current_type == HEX_NUMBER_TOKEN)
    {
        const int8_t *number_text =
            token_arr[*see_token].token_value_data.token_name;
        uint16_t number_index = 2;
        uint8_t number_base = 10;
        uint64_t number_value = 0;
        uint8_t has_digit = FALSE;

        if (number_text[0] != '0')
        {
            return psr_result(NOT_EXPRESSION, PSR_NO_NODE);
        }

        if (number_text[1] == 'b')
        {
            number_base = 2;
        }
        else if (number_text[1] == 'x')
        {
            number_base = 16;
        }
        else if (number_text[1] != 'd' && number_text[1] != 'f')
        {
            return psr_result(NOT_EXPRESSION, PSR_NO_NODE);
        }

        for (; number_index < 32 && number_text[number_index] != '\0'; number_index++)
        {
            uint8_t number_digit;
            int8_t number_char = number_text[number_index];

            if (number_char >= '0' && number_char <= '9')
            {
                number_digit = (uint8_t)(number_char - '0');
            }
            else if (number_char >= 'a' && number_char <= 'f')
            {
                number_digit = (uint8_t)(number_char - 'a' + 10);
            }
            else if (number_char >= 'A' && number_char <= 'F')
            {
                number_digit = (uint8_t)(number_char - 'A' + 10);
            }
            else
            {
                return psr_result(NOT_EXPRESSION, PSR_NO_NODE);
            }

            if (number_digit >= number_base ||
                number_value > (((uint64_t)-1) - number_digit) / number_base)
            {
                return psr_result(NOT_EXPRESSION, PSR_NO_NODE);
            }

            has_digit = TRUE;
            number_value = number_value * number_base + number_digit;
        }

        if (number_index == 32 || !has_digit)
        {
            return psr_result(NOT_EXPRESSION, PSR_NO_NODE);
        }

        psr_init_node(&new_node, NUM_NODE);
        new_node.value.num = number_value;
        (*see_token)++;
        new_psr_state = psr_store_node(node_arr, new_node);
    }
    // 문자열 처리
    else if (current_type == STRING_TOKEN)
    {
        psr_init_node(&new_node, STRING_NODE);
        new_node.value.sym = *see_token;
        (*see_token)++;
        new_psr_state = psr_store_node(node_arr, new_node);
    }
    else
    {
        return psr_result(NOT_EXPRESSION, PSR_NO_NODE);
    }

    if (new_psr_state.error_code != PARSER_OK)
    {
        return new_psr_state;
    }

    uint16_t expression_pos = new_psr_state.pos;

    // 함수 호출, 배열 인덱스, 멤버 접근 처리
    // 변수 뒤에 붙는 호출, 배열 인덱스, 멤버를 이어서 확인한다
    while (*see_token < token_number)
    {
        current_type = psr_token_type_at(token_arr, *see_token, token_number);

        if (current_type == SO_BRACKET_TOKEN)
        {
            (*see_token)++;

            uint16_t arg_head = PSR_NO_NODE;
            uint16_t arg_tail = PSR_NO_NODE;

            if (psr_token_type_at(token_arr, *see_token, token_number) != SC_BRACKET_TOKEN)
            {
                // 인자 노드 연결
                while (TRUE)
                {
                    check_Parser arg_state = psr_expr(
                        node_arr,
                        token_arr,
                        see_token,
                        token_number,
                        1);
                    if (arg_state.error_code != PARSER_OK)
                    {
                        return arg_state;
                    }

                    psr_init_node(&new_node, ARG_NODE);
                    new_node.left = arg_state.pos;
                    check_Parser arg_node_state = psr_store_node(node_arr, new_node);
                    if (arg_node_state.error_code != PARSER_OK)
                    {
                        return arg_node_state;
                    }

                    if (arg_head == PSR_NO_NODE)
                    {
                        arg_head = arg_node_state.pos;
                    }
                    else
                    {
                        node_arr[arg_tail].right = arg_node_state.pos;
                    }
                    arg_tail = arg_node_state.pos;

                    if (psr_token_type_at(token_arr, *see_token, token_number) != COMMA_TOKEN)
                    {
                        break;
                    }
                    (*see_token)++;
                }
            }

            if (psr_token_type_at(token_arr, *see_token, token_number) != SC_BRACKET_TOKEN)
            {
                return psr_result(NOT_SC_BRACKET, PSR_NO_NODE);
            }
            (*see_token)++;

            psr_init_node(&new_node, CALL_NODE);
            new_node.left = expression_pos;
            new_node.right = arg_head;
            check_Parser call_state = psr_store_node(node_arr, new_node);
            if (call_state.error_code != PARSER_OK)
            {
                return call_state;
            }
            expression_pos = call_state.pos;
        }
        else if (current_type == BO_BRACKET_TOKEN)
        {
            (*see_token)++;

            check_Parser index_state = psr_expr(
                node_arr,
                token_arr,
                see_token,
                token_number,
                1);
            if (index_state.error_code != PARSER_OK)
            {
                return index_state;
            }
            if (psr_token_type_at(token_arr, *see_token, token_number) != BC_BRACKET_TOKEN)
            {
                return psr_result(NOT_BC_BRACKET, PSR_NO_NODE);
            }
            (*see_token)++;

            psr_init_node(&new_node, INDEX_NODE);
            new_node.left = expression_pos;
            new_node.right = index_state.pos;
            check_Parser index_node_state = psr_store_node(node_arr, new_node);
            if (index_node_state.error_code != PARSER_OK)
            {
                return index_node_state;
            }
            expression_pos = index_node_state.pos;
        }
        else if (current_type == DOT_TOKEN)
        {
            (*see_token)++;
            if (psr_token_type_at(token_arr, *see_token, token_number) != VAL_TOKEN)
            {
                return psr_result(NOT_VAR_NAME, PSR_NO_NODE);
            }

            psr_init_node(&new_node, MEMBER_NODE);
            new_node.left = expression_pos;
            new_node.value.sym = *see_token;
            (*see_token)++;
            check_Parser member_state = psr_store_node(node_arr, new_node);
            if (member_state.error_code != PARSER_OK)
            {
                return member_state;
            }
            expression_pos = member_state.pos;
        }
        else
        {
            break;
        }
    }

    // 왼쪽 피연산자 파싱이 끝났으므로 연산자 우선순위를 확인한다
    uint16_t left_pos = expression_pos;
    // 연산자가 아니라면 표현식 파싱을 마침
    while (*see_token < token_number)
    {
        uint16_t operator_type = psr_token_type_at(token_arr, *see_token, token_number);
        uint8_t precedence = 0;

        // 연산자 우선순위 확인
        switch (operator_type)
        {
        case ASSIGN_TOKEN:
            precedence = 1;
            break;
        case LOGICAL_OR_TOKEN:
            precedence = 2;
            break;
        case LOGICAL_AND_TOKEN:
            precedence = 3;
            break;
        case OR_TOKEN:
            precedence = 4;
            break;
        case XOR_TOKEN:
            precedence = 5;
            break;
        case AND_TOKEN:
            precedence = 6;
            break;
        case EQUAL_TOKEN:
        case NOT_EQUAL_TOKEN:
            precedence = 7;
            break;
        case LESS_TOKEN:
        case LESS_EQUAL_TOKEN:
        case GREATER_TOKEN:
        case GREATER_EQUAL_TOKEN:
            precedence = 8;
            break;
        case PLUS_TOKEN:
        case MINU_TOKEN:
            precedence = 9;
            break;
        case MULTI_TOKEN:
        case DIV_TOKEN:
            precedence = 10;
            break;
        default:
            break;
        }

        if (precedence < min_precedence || precedence == 0)
        {
            break;
        }

        if (operator_type == ASSIGN_TOKEN &&
            node_arr[left_pos].node_type != VAR_NODE &&
            node_arr[left_pos].node_type != INDEX_NODE &&
            node_arr[left_pos].node_type != MEMBER_NODE)
        {
            return psr_result(NOT_OP, PSR_NO_NODE);
        }

        // 연산자 토큰 소비
        (*see_token)++;
        check_Parser right_state = psr_expr(
            node_arr,
            token_arr,
            see_token,
            token_number,
            operator_type == ASSIGN_TOKEN ? precedence : (uint8_t)(precedence + 1));
        if (right_state.error_code != PARSER_OK)
        {
            return right_state;
        }

        uint8_t node_type = BINARY_OP_NODE;
        if (operator_type == ASSIGN_TOKEN)
        {
            node_type = ASSIGN_NODE;
        }
        else if (operator_type == EQUAL_TOKEN ||
                 operator_type == NOT_EQUAL_TOKEN ||
                 operator_type == LESS_TOKEN ||
                 operator_type == LESS_EQUAL_TOKEN ||
                 operator_type == GREATER_TOKEN ||
                 operator_type == GREATER_EQUAL_TOKEN)
        {
            node_type = COMPARE_OP_NODE;
        }
        else if (operator_type == LOGICAL_AND_TOKEN ||
                 operator_type == LOGICAL_OR_TOKEN)
        {
            node_type = LOGICAL_OP_NODE;
        }

        // 연산자 노드 생성
        psr_init_node(&new_node, node_type);
        new_node.value.op = (uint8_t)operator_type;
        new_node.left = left_pos;
        new_node.right = right_state.pos;
        new_psr_state = psr_store_node(node_arr, new_node);
        if (new_psr_state.error_code != PARSER_OK)
        {
            return new_psr_state;
        }
        left_pos = new_psr_state.pos;
    }

    return psr_result(PARSER_OK, left_pos);
}
