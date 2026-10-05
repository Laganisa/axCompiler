#ifndef _COMPIL_PARSER_H_
#define _COMPIL_PARSER_H_

#include "kernel.h"
#include "defs.h"
#include "lexer.h"
#include "ast_node.h"

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

extern uint16_t global_node_pos;

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

static inline uint8_t psr_names_equal(
    const int8_t *name_left,
    const int8_t *name_right)
{
    uint16_t name_index = 0;
    while (name_index < 32 &&
           name_left[name_index] == name_right[name_index])
    {
        if (name_left[name_index] == '\0')
        {
            return TRUE;
        }
        name_index++;
    }
    return FALSE;
}

static inline uint16_t psr_symbol_id(
    const token *token_arr,
    uint16_t token_pos)
{
    for (uint16_t name_index = 0; name_index < token_pos; name_index++)
    {
        if (token_arr[name_index].token_type == VAL_TOKEN &&
            psr_names_equal(
                token_arr[name_index].token_value_data.token_name,
                token_arr[token_pos].token_value_data.token_name))
        {
            return name_index;
        }
    }
    return token_pos;
}

static inline uint8_t psr_number_value(
    const token *token_arr,
    uint16_t token_pos,
    uint64_t *number_value)
{
    const int8_t *number_text = token_arr[token_pos].token_value_data.token_name;

    uint16_t number_index = 2;
    uint8_t number_base = 10;
    uint64_t number_result = 0;
    uint8_t has_digit = FALSE;

    if (number_text[0] != '0')
    {
        return FALSE;
    }

    if (number_text[1] == 'b')
    {
        number_base = 2;
    }
    else if (number_text[1] == 'x')
    {
        number_base = 16;
    }
    else if (number_text[1] == 'd' || number_text[1] == 'f')
    {
        number_base = 10;
    }
    else
    {
        return FALSE;
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
            return FALSE;
        }

        if (number_digit >= number_base ||
            number_result > (((uint64_t)-1) - number_digit) / number_base)
        {
            return FALSE;
        }
        has_digit = TRUE;
        number_result = number_result * number_base + number_digit;
    }

    if (number_index == 32 || !has_digit)
    {
        return FALSE;
    }

    *number_value = number_result;
    return TRUE;
}

static inline uint8_t psr_op_precedence(uint16_t token_type)
{
    // 연산자 우선순위 확인
    switch (token_type)
    {
    case ASSIGN_TOKEN:
        return 1;
    case LOGICAL_OR_TOKEN:
        return 2;
    case LOGICAL_AND_TOKEN:
        return 3;
    case OR_TOKEN:
        return 4;
    case XOR_TOKEN:
        return 5;
    case AND_TOKEN:
        return 6;
    case EQUAL_TOKEN:
    case NOT_EQUAL_TOKEN:
        return 7;
    case LESS_TOKEN:
    case LESS_EQUAL_TOKEN:
    case GREATER_TOKEN:
    case GREATER_EQUAL_TOKEN:
        return 8;
    case PLUS_TOKEN:
    case MINU_TOKEN:
        return 9;
    case MULTI_TOKEN:
    case DIV_TOKEN:
        return 10;
    default:
        return 0;
    }
}

static inline check_Parser psr_expr_prec(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number,
    uint8_t min_precedence);

static inline check_Parser psr_primary(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number);

static inline check_Parser psr_primary(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number)
{
    // 재귀함수 호출에 쓰일 리턴 구조체
    check_Parser new_psr_state = psr_result(UNEXPECTED_TOKEN, PSR_NO_NODE);

    // 배열에 쓸 새로운 노드
    ast_node new_node = {0};
    uint16_t current_type = psr_token_type_at(token_arr, *see_token, token_number);

    // 단항 연산자 처리
    if (current_type == PLUS_TOKEN ||
        current_type == MINU_TOKEN ||
        current_type == NOT_TOKEN ||
        current_type == PTR_TOKEN)
    {
        uint16_t operator_type = current_type;

        (*see_token)++;

        check_Parser now_psr_state = psr_primary(
            node_arr,
            token_arr,
            see_token,
            token_number);

        if (now_psr_state.error_code != PARSER_OK)
        {
            return now_psr_state;
        }

        psr_init_node(&new_node, operator_type == PTR_TOKEN ? PTR_NODE : UNARY_OP_NODE);

        new_node.value.op = (uint8_t)operator_type;
        new_node.right = now_psr_state.pos;
        new_psr_state = psr_store_node(node_arr, new_node);
    }
    // 괄호가 있다면
    else if (current_type == SO_BRACKET_TOKEN)
    {
        (*see_token)++;

        new_psr_state = psr_expr_prec(
            node_arr,
            token_arr,
            see_token,
            token_number,
            1);

        // 재귀로 받은 노드 확인
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

        psr_init_node(&new_node, VAR_NODE);

        new_node.value.sym = psr_symbol_id(token_arr, name_pos);

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
        uint16_t number_pos = *see_token;
        uint64_t number_value = 0;

        if (!psr_number_value(token_arr, number_pos, &number_value))
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
                    check_Parser arg_state = psr_expr_prec(
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

            check_Parser index_state = psr_expr_prec(
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
            new_node.value.sym = psr_symbol_id(token_arr, *see_token);
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

    return psr_result(PARSER_OK, expression_pos);
}

static inline check_Parser psr_expr_prec(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number,
    uint8_t min_precedence)
{
    // 왼쪽 피연산자 파싱
    // 재귀함수 호출에 쓰일 리턴 구조체
    check_Parser new_psr_state = psr_primary(
        node_arr,
        token_arr,
        see_token,
        token_number);
    // 재귀로 받은 노드 확인
    if (new_psr_state.error_code != PARSER_OK)
    {
        return new_psr_state;
    }

    uint16_t left_pos = new_psr_state.pos;
    // while에서 쓸 변수
    // 연산자가 아니라면 표현식 파싱을 마침
    while (*see_token < token_number)
    {
        uint16_t operator_type = psr_token_type_at(token_arr, *see_token, token_number);
        uint8_t precedence = psr_op_precedence(operator_type);
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
        check_Parser right_state = psr_expr_prec(
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
        ast_node new_node = {0};
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

// 파서 재귀 함수들
check_Parser psr_expr(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number);

check_Parser psr_stmt(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number);

check_Parser psr_decl(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number);

check_Parser psr_block(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number);

#endif