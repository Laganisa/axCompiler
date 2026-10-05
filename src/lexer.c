#include "lexer.h"
#include "string.h"
#include "token.h"
#include "inline.h"
#include "call.h"

typedef struct keyword_t
{
    const char *keyword_name_text;
    enum TOKEN_TYPE keyword_token_type;
} keyword_t;

static const keyword_t lexer_keyword_arr[] =
    {
        {"if", IF_TOKEN},
        {"else", ELSE_TOKEN},
        {"for", FOR_TOKEN},
        {"while", WHILE_TOKEN},
        {"return", RETURN_TOKEN},
        {"uint8", UINT8_TOKEN},
        {"uint16", UINT16_TOKEN},
        {"uint32", UINT32_TOKEN},
        {"uint64", UINT64_TOKEN},
        {"int8", INT8_TOKEN},
        {"int16", INT16_TOKEN},
        {"int32", INT32_TOKEN},
        {"int64", INT64_TOKEN},
        {"(", SO_BRACKET_TOKEN},
        {")", SC_BRACKET_TOKEN},
        {"{", MO_BRACKET_TOKEN},
        {"}", MC_BRACKET_TOKEN}};

static uint8_t lexer_append_token(
    token *lexer_token_arr,
    uint16_t *lexer_token_cnt_p,
    token lexer_token_value)
{
    if (*lexer_token_cnt_p >= MAX_TOKEN_ARR - 1)
    {
        return 0;
    }

    lexer_token_arr[*lexer_token_cnt_p] = lexer_token_value;
    (*lexer_token_cnt_p)++;
    return 1;
}

static uint8_t lexer_is_hex_digit(int8_t lxr_src_char)
{
    return is_hex_number(lxr_src_char) ||
           (lxr_src_char >= 'a' && lxr_src_char <= 'f');
}

/*
    렉서
    렉서 토큰 배열에 한 줄 문자 입력을 받음
*/
check_Lexer lexer(
    token *lexer_token_arr,
    const int8_t lxr_src_line[64],
    uint16_t lxr_src_line_num,
    uint16_t lexer_token_cnt)
{
    write(0, "%s", "enter lexer\n");

    check_Lexer lxr_op_result = {0};
    lxr_op_result.lxr_error_code = LEXER_OK;

    // 에러 처리
    if (
        lexer_token_arr == 0 ||
        lxr_src_line == 0 ||
        lexer_token_cnt >= MAX_TOKEN_ARR)
    {
        lxr_op_result.lxr_error_code = MAX_TOKEN_ERROR;
        lxr_op_result.lxr_past_token_cnt = lexer_token_cnt;
        return lxr_op_result;
    }

    uint16_t lxr_src_index = 0;

    // 어휘 분석 시작
    while (lxr_src_index < 64 &&
           lxr_src_line[lxr_src_index] != '\0')
    {
        int8_t lxr_src_char = lxr_src_line[lxr_src_index];
        token lxr_curr_token = {0};
        lxr_curr_token.token_src_column = (uint8_t)lxr_src_index;
        lxr_curr_token.token_src_line = lxr_src_line_num;

        // 넘김의 어휘 분석
        if (lxr_src_char == ' ' || lxr_src_char == '\t' ||
            lxr_src_char == '\n' || lxr_src_char == '\r')
        {
            lxr_src_index++;
            continue;
        }

        // 명칭의 어휘 분석
        if (lxr_src_char == '_' || is_letter(lxr_src_char))
        {
            uint8_t lexer_identifier_name_len = 0;
            lxr_curr_token.token_type = VAL_TOKEN;

            while (lxr_src_index < 64 &&
                   lexer_identifier_name_len <
                       sizeof(lxr_curr_token.token_value_data.token_name) - 1 &&
                   (lxr_src_line[lxr_src_index] == '_' ||
                    is_letter(lxr_src_line[lxr_src_index]) ||
                    is_dec_number(lxr_src_line[lxr_src_index])))
            {
                lxr_curr_token.token_value_data.token_name[lexer_identifier_name_len] =
                    lxr_src_line[lxr_src_index];
                lexer_identifier_name_len++;
                lxr_src_index++;
            }

            if (lxr_src_index < 64 &&
                (lxr_src_line[lxr_src_index] == '_' ||
                 is_letter(lxr_src_line[lxr_src_index]) ||
                 is_dec_number(lxr_src_line[lxr_src_index])))
            {
                lxr_op_result.lxr_error_code = NAME_OVER_ERROR;
                break;
            }

            lxr_curr_token.token_value_data.token_name[lexer_identifier_name_len] = '\0';

            for (size_t lxr_keyword_index = 0;
                 lxr_keyword_index <
                 sizeof(lexer_keyword_arr) / sizeof(lexer_keyword_arr[0]);
                 lxr_keyword_index++)
            {
                if (axlib_strcmp(
                        (const char *)lxr_curr_token.token_value_data.token_name,
                        lexer_keyword_arr[lxr_keyword_index].keyword_name_text) == 0)
                {
                    lxr_curr_token.token_type =
                        lexer_keyword_arr[lxr_keyword_index].keyword_token_type;
                    break;
                }
            }

            if (!lexer_append_token(
                    lexer_token_arr,
                    &lexer_token_cnt,
                    lxr_curr_token))
            {
                lxr_op_result.lxr_error_code = MAX_TOKEN_ERROR;
                break;
            }
            continue;
        }

        // 숫자의 어휘 분석
        if (lxr_src_char == '0')
        {
            lxr_src_index++;

            // ! 이거 if 문으로 바꾸기
            int8_t lexer_num_prefix =
                lxr_src_index < 64
                    ? lxr_src_line[lxr_src_index]
                    : '\0';

            uint8_t lexer_digit_cnt = 0;
            uint8_t lexer_num_type = 0;

            lxr_curr_token.token_value_data.token_name[0] = '0';
            lxr_curr_token.token_value_data.token_name[1] = lexer_num_prefix;

            if (lexer_num_prefix == 'b')
            {
                lexer_num_type = BIN_NUMBER_TOKEN;
            }
            else if (lexer_num_prefix == 'd')
            {
                lexer_num_type = DEC_NUMBER_TOKEN;
            }
            else if (lexer_num_prefix == 'x')
            {
                lexer_num_type = HEX_NUMBER_TOKEN;
            }
            // 숫자 접두사
            else if (lexer_num_prefix == 'f')
            {
                lexer_num_type = FLOAT_NUMBER_TOKEN;
            }
            // 숫자 접두사 에러
            else
            {
                lxr_op_result.lxr_error_code = PREFIX_ERROR;
                break;
            }

            lxr_src_index++;
            while (lxr_src_index < 64 &&
                   lxr_src_line[lxr_src_index] != '\0')
            {
                lxr_src_char =
                    lxr_src_line[lxr_src_index];

                // ! 이거 if 문으로 바꾸기
                uint8_t lxr_is_valid_digit =
                    lexer_num_type == BIN_NUMBER_TOKEN
                        ? is_bin_number(lxr_src_char)
                    : lexer_num_type == DEC_NUMBER_TOKEN
                        ? is_dec_number(lxr_src_char)
                        : lexer_is_hex_digit(lxr_src_char);

                if (!lxr_is_valid_digit)
                {
                    break;
                }

                if (lexer_digit_cnt >= 14)
                {
                    lxr_op_result.lxr_error_code = NUMBER_OVER_ERROR;
                    break;
                }

                lxr_curr_token.token_value_data.token_name[2 + lexer_digit_cnt] =
                    lxr_src_char;
                lexer_digit_cnt++;
                lxr_src_index++;
            }

            if (lxr_op_result.lxr_error_code != LEXER_OK)
            {
                break;
            }
            if (lexer_digit_cnt == 0)
            {
                lxr_op_result.lxr_error_code = NUMBER_VOID_ERROR;
                break;
            }
            if (lxr_src_index < 64 &&
                (is_letter(lxr_src_line[lxr_src_index]) ||
                 is_dec_number(lxr_src_line[lxr_src_index]) ||
                 lxr_src_line[lxr_src_index] == '_'))
            {
                lxr_op_result.lxr_error_code = NUMBER_MAKE_ERROR;
                break;
            }

            lxr_curr_token.token_value_data.token_name[2 + lexer_digit_cnt] = '\0';
            lxr_curr_token.token_type = lexer_num_type;

            if (!lexer_append_token(
                    lexer_token_arr,
                    &lexer_token_cnt,
                    lxr_curr_token))
            {
                lxr_op_result.lxr_error_code = MAX_TOKEN_ERROR;
                break;
            }
            continue;
        }

        // 나누기 및 주석의 어휘 분석
        if (lxr_src_char == '/')
        {
            lxr_src_index++;
            if (lxr_src_index < 64 &&
                lxr_src_line[lxr_src_index] == '/')
            {
                break;
            }
            lxr_curr_token.token_type = DIV_TOKEN;
        }

        // 문장의 어휘 분석
        else if (lxr_src_char == '"')
        {
            uint8_t lexer_literal_string_len = 0;
            lxr_src_index++;

            while (lxr_src_index < 64 &&
                   lxr_src_line[lxr_src_index] != '\0' &&
                   lxr_src_line[lxr_src_index] != '"' &&
                   lxr_src_line[lxr_src_index] != '\n' &&
                   lxr_src_line[lxr_src_index] != '\r')
            {
                if (lexer_literal_string_len >=
                    sizeof(lxr_curr_token.token_value_data.token_name) - 1)
                {
                    lxr_op_result.lxr_error_code = STRING_MAKE_ERROR;
                    break;
                }
                lxr_curr_token.token_value_data.token_name[lexer_literal_string_len] =
                    lxr_src_line[lxr_src_index];
                lexer_literal_string_len++;
                lxr_src_index++;
            }

            if (lxr_op_result.lxr_error_code != LEXER_OK)
            {
                break;
            }
            if (lxr_src_index >= 64 ||
                lxr_src_line[lxr_src_index] != '"')
            {
                lxr_op_result.lxr_error_code = STRING_MAKE_ERROR;
                break;
            }

            lxr_src_index++;
            lxr_curr_token.token_value_data.token_name[lexer_literal_string_len] = '\0';
            lxr_curr_token.token_type = STRING_TOKEN;
        }
        else
        {
            switch (lxr_src_char)
            {
            // 산술 연산자의 어휘 분석
            case '+':
                lxr_curr_token.token_type = PLUS_TOKEN;
                break;
            case '-':
                lxr_curr_token.token_type = MINU_TOKEN;
                break;
            case '*':
                lxr_curr_token.token_type = MULTI_TOKEN;
                break;

            // 비교 연산자의 어휘 분석
            case '!':
                if (lxr_src_index + 1 < 64 &&
                    lxr_src_line[lxr_src_index + 1] == '=')
                {
                    lxr_curr_token.token_type = NOT_EQUAL_TOKEN;
                    lxr_src_index++;
                }
                else
                {
                    lxr_curr_token.token_type = NOT_TOKEN;
                }
                break;
            case '=':
                if (lxr_src_index + 1 < 64 &&
                    lxr_src_line[lxr_src_index + 1] == '=')
                {
                    lxr_curr_token.token_type = EQUAL_TOKEN;
                    lxr_src_index++;
                }
                else
                {
                    lxr_curr_token.token_type = ASSIGN_TOKEN;
                }
                break;
            case '<':
                if (lxr_src_index + 1 < 64 &&
                    lxr_src_line[lxr_src_index + 1] == '=')
                {
                    lxr_curr_token.token_type = LESS_EQUAL_TOKEN;
                    lxr_src_index++;
                }
                else
                {
                    lxr_curr_token.token_type = LESS_TOKEN;
                }
                break;
            case '>':
                if (lxr_src_index + 1 < 64 &&
                    lxr_src_line[lxr_src_index + 1] == '=')
                {
                    lxr_curr_token.token_type = GREATER_EQUAL_TOKEN;
                    lxr_src_index++;
                }
                else
                {
                    lxr_curr_token.token_type = GREATER_TOKEN;
                }
                break;

            // 논리 및 비트 연산자의 어휘 분석
            case '&':
                if (lxr_src_index + 1 < 64 &&
                    lxr_src_line[lxr_src_index + 1] == '&')
                {
                    lxr_curr_token.token_type = LOGICAL_AND_TOKEN;
                    lxr_src_index++;
                }
                else
                {
                    lxr_curr_token.token_type = AND_TOKEN;
                }
                break;
            case '|':
                if (lxr_src_index + 1 < 64 &&
                    lxr_src_line[lxr_src_index + 1] == '|')
                {
                    lxr_curr_token.token_type = LOGICAL_OR_TOKEN;
                    lxr_src_index++;
                }
                else
                {
                    lxr_curr_token.token_type = OR_TOKEN;
                }
                break;
            case '^':
                lxr_curr_token.token_type = XOR_TOKEN;
                break;

            // 포인터의 어휘 분석
            case '$':
                lxr_curr_token.token_type = PTR_TOKEN;
                break;

            // 괄호의 어휘 분석
            case '(':
                lxr_curr_token.token_type = SO_BRACKET_TOKEN;
                break;
            case ')':
                lxr_curr_token.token_type = SC_BRACKET_TOKEN;
                break;
            case '{':
                lxr_curr_token.token_type = MO_BRACKET_TOKEN;
                break;
            case '}':
                lxr_curr_token.token_type = MC_BRACKET_TOKEN;
                break;
            case '[':
                lxr_curr_token.token_type = BO_BRACKET_TOKEN;
                break;
            case ']':
                lxr_curr_token.token_type = BC_BRACKET_TOKEN;
                break;

            // 구분자의 어휘 분석
            case ';':
                lxr_curr_token.token_type = SEMI_TOKEN;
                break;
            case ',':
                lxr_curr_token.token_type = COMMA_TOKEN;
                break;
            case '.':
                lxr_curr_token.token_type = DOT_TOKEN;
                break;

            // 에러 처리
            default:
                lxr_op_result.lxr_error_code = NOT_SYMBOL_ERROR;
                break;
            }

            if (lxr_op_result.lxr_error_code != LEXER_OK)
            {
                break;
            }
            lxr_src_index++;
        }

        if (!lexer_append_token(
                lexer_token_arr,
                &lexer_token_cnt,
                lxr_curr_token))
        {
            lxr_op_result.lxr_error_code = MAX_TOKEN_ERROR;
            break;
        }
    }

    // 종료 토큰 붙여주기
    token lexer_end_token = {0};
    lexer_end_token.token_type = END_TOKEN;
    lexer_token_arr[lexer_token_cnt] = lexer_end_token;
    lxr_op_result.lxr_past_token_cnt = lexer_token_cnt;

    write(0, "%s", "exit lexer\n");
    return lxr_op_result;
}
