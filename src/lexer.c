#include "lexer.h"
#include "string.h"

#pragma region inline

static inline uint8_t is_letter(int l)
{
    if (l >= 0x41 && l <= 0x5A)
    {
        return 1;
    }
    if (l >= 0x61 && l <= 0x7A)
    {
        return 1;
    }
    return 0;
}

static inline uint8_t is_dec_number(int n)
{
    if (n >= 48 && n <= 57)
    {
        return 1;
    }

    return 0;
}

static inline uint8_t is_bin_number(int n)
{
    if (n == '0' || n == '1')
    {
        return 1;
    }
    return 0;
}

static inline uint8_t is_hex_number(int n)
{
    if (n >= '0' && n <= '9')
    {
        return 1;
    }
    if (n >= 'A' && n <= 'F')
    {
        return 1;
    }
    return 0;
}

#pragma endregion

check_L lexer(token *token_arr, int8_t arr[64], uint8_t see_line, uint16_t see_token)
{
    // 리턴할 (어디를 읽을지, 에러코드)튜플
    check_L result;

    // 된다고 한 다음 에러로 바꾸기
    result.error_code = CAN_GO;

    // 입력 줄이 들어오면 토큰으로 분리하는 로직
    uint8_t see = 0;
    while (see <= 63 && see_token < MAX_TOKEN_ARR)
    {
        // 토큰 배열에 넣을
        token now_token;

        // 넘김의 인식
        if (arr[see] == ' ' || arr[see] == '\t' || arr[see] == '\n')
        {
            see++;
            continue;
        }
        // 명칭의 인식
        else if (arr[see] == 0x5f || is_letter(arr[see]))
        {
            uint8_t cnt = 0;
            now_token.col = see;
            now_token.data.name[cnt] = arr[see];
            see++;

            // 줄의 끝까지
            while (see <= 63 && cnt < 31)
            {

                if (arr[see] == 0x5f || is_letter(arr[see]) || is_dec_number(arr[see]))
                {
                    now_token.data.name[cnt] = arr[see];
                    see++;
                }
                else
                {
                    break;
                }

                cnt++;
            }

            now_token.offset = see_line;
            now_token.data.name[cnt] = '\0';
            // 예약어 정리

            for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++)
            {
                if (axlib_strcmp(now_token.data.name, keywords[i].name) == 0)
                {
                    now_token.type = keywords[i].type;
                    break;
                }
            }

            // 토큰에 넣어주기
            token_arr[see_token] = now_token;
            see_token++;
        }
        // 수의 인식
        else if (arr[see] == '0')
        {
            see++;
            now_token.col = see;

            now_token.data.name[0] = '0';
            // 이진수
            if (arr[see] == 'b')
            {
                uint8_t cnt = 2;
                see++;
                now_token.data.name[1] = 'b';
                // 16자가 되지 않을 때까지
                while (see <= 63 && cnt <= 16)
                {
                    if (is_bin_number(arr[see]))
                    {
                        now_token.data.name[cnt] = arr[see];
                        see++;
                    }
                    else
                    {
                        // 나가기
                        break;
                    }
                }

                if (cnt == 17)
                {
                    result.error_code = NUMBER_OVER_ERROR;
                    break;
                }
                else if (cnt == 3)
                {
                    result.error_code = NUMBER_VOID_ERROR;
                    break;
                }

                now_token.type = BIN_NUMBER_TOKEN;
            }
            // 십진수
            else if (arr[see] == 'd')
            {
                uint8_t cnt = 2;
                see++;
                now_token.data.name[1] = 'd';
                // 16자가 되지 않을 때까지
                while (see <= 63 && cnt <= 16)
                {
                    if (is_dec_number(arr[see]))
                    {
                        now_token.data.name[cnt] = arr[see];
                        see++;
                    }
                    else
                    {
                        // 나가기
                        break;
                    }
                }

                if (cnt == 17)
                {
                    result.error_code = NUMBER_OVER_ERROR;
                    break;
                }
                else if (cnt == 3)
                {
                    result.error_code = NUMBER_VOID_ERROR;
                    break;
                }
                now_token.type = DEC_NUMBER_TOKEN;
            }
            // 16진수
            else if (arr[see] == 'x')
            {
                uint8_t cnt = 2;
                see++;
                now_token.data.name[1] = 'x';
                // 16자가 되지 않을 때까지
                while (see <= 63 && cnt <= 16)
                {
                    if (is_hex_number(arr[see]))
                    {
                        now_token.data.name[cnt] = arr[see];
                        see++;
                    }
                    else
                    {
                        // 나가기
                        break;
                    }
                }

                if (cnt == 17)
                {
                    result.error_code = NUMBER_OVER_ERROR;
                    break;
                }
                else if (cnt == 3)
                {
                    result.error_code = NUMBER_VOID_ERROR;
                    break;
                }
                now_token.type = HEX_NUMBER_TOKEN;
            }
            // 실수(정밀도, 배정밀도)
            else if (arr[see] == 'f')
            {
                uint8_t cnt = 2;
                see++;
                now_token.data.name[1] = 'f';
                // 16자가 되지 않을 때까지
                while (see <= 63 && cnt <= 16)
                {
                    if (is_hex_number(arr[see]))
                    {
                        now_token.data.name[cnt] = arr[see];
                        see++;
                    }
                    else
                    {
                        // 나가기
                        break;
                    }
                }

                if (cnt == 17)
                {
                    result.error_code = NUMBER_OVER_ERROR;
                    break;
                }
                else if (cnt == 3)
                {
                    result.error_code = NUMBER_VOID_ERROR;
                    break;
                }
                now_token.type = FLOAT_NUMBER_TOKEN;
            }
            // 에러
            else
            {
                result.error_code = PREFIX_ERROR; // 접두사 선언 에러
                break;
            }

            // name에 저장했던거 숫자로 바꾸기 파서떄

            now_token.offset = see_line;
            token_arr[see_token] = now_token;
            see_token++;
        }
        // 주석의 인식(// \n이 나올때까지 토큰을 생성하지 않기), 나누기 로직
        else if (arr[see] == '/')
        {
            see++;
            if (arr[see] == '/')
            {
                see++;
                // 주석의 처리
                while (see <= 63 && arr[see] != 0x0A)
                {
                    see++;
                }
            }
            else
            {
                // 나눗셈으로 토큰
                now_token.col = see;
                now_token.type = DIV_TOKEN;
                now_token.offset = see_line;
                token_arr[see_token] = now_token;
                see_token++;
            }
        }
        // 문자의 인식
        else if (arr[see] == '"')
        {
            uint8_t cnt = 0;
            now_token.col = see;
            see++;
            // 다시 34가 나올때가지 while
            while (arr[see] < 63 && cnt < 31)
            {
                if (arr[see] == '"')
                {
                    see++;
                    break;
                }
                else
                {
                    now_token.data.name[cnt] = arr[see];
                    cnt++;
                    see++;
                }
            }
            now_token.data.name[cnt] = '\0';
            now_token.offset = see_line;
            token_arr[see_token] = now_token;
            see_token++;
        }
        // 연산자의 인식(+,-,*)
        else if (arr[see] == '+')
        {
            // 더하기 로직
            now_token.col = see;
            now_token.type = PLUS_TOKEN;
            now_token.offset = see_line;
            token_arr[see_token] = now_token;
            see_token++;
            see++;
        }
        else if (arr[see] == '-')
        {
            // 빼기 로직
            now_token.col = see;
            now_token.type = MINU_TOKEN;
            now_token.offset = see_line;
            token_arr[see_token] = now_token;
            see_token++;
            see++;
        }
        else if (arr[see] == '*')
        {
            // 곱하기 로직
            now_token.col = see;
            now_token.type = MULTI_TOKEN;
            now_token.offset = see_line;
            token_arr[see_token] = now_token;
            see_token++;
            see++;
        }
        else if (arr[see] == '!')
        {
            // 부정
            now_token.col = see;
            now_token.type = NOT_TOKEN;
            now_token.offset = see_line;
            token_arr[see_token] = now_token;
            see_token++;
            see++;
        }
        else if (arr[see] == '&')
        {
            // 그리고
            now_token.col = see;
            now_token.type = AND_TOKEN;
            now_token.offset = see_line;
            token_arr[see_token] = now_token;
            see_token++;
            see++;
        }
        else if (arr[see] == '|')
        {
            // 또는
            now_token.col = see;
            now_token.type = OR_TOKEN;
            now_token.offset = see_line;
            token_arr[see_token] = now_token;
            see_token++;
            see++;
        }
        else if (arr[see] == '^')
        {
            // 베타적 논리합
            now_token.col = see;
            now_token.type = XOR_TOKEN;
            now_token.offset = see_line;
            token_arr[see_token] = now_token;
            see_token++;
            see++;
        }
        // 구분자 및 괄호의 인식
        else if (arr[see] == '(')
        {
            // 열린 괄호
            now_token.col = see;
            now_token.type = SO_BRACKET_TOKEN;
            now_token.offset = see_line;
            token_arr[see_token] = now_token;
            see_token++;
            see++;
        }
        else if (arr[see] == ')')
        {
            // 닫힌 괄호
            now_token.col = see;
            now_token.type = SC_BRACKET_TOKEN;
            now_token.offset = see_line;
            token_arr[see_token] = now_token;
            see_token++;
            see++;
        }
        else if (arr[see] == '{')
        {
            // 열린 중괄호
            now_token.col = see;
            now_token.type = MO_BRACKET_TOKEN;
            now_token.offset = see_line;
            token_arr[see_token] = now_token;
            see_token++;
            see++;
        }
        else if (arr[see] == '}')
        {
            // 닫힌 중괄호
            now_token.col = see;
            now_token.type = MC_BRACKET_TOKEN;
            now_token.offset = see_line;
            token_arr[see_token] = now_token;
            see_token++;
            see++;
        }
        else if (arr[see] == ';')
        {
            now_token.col = see;
            now_token.type = SEMI_TOKEN;
            now_token.offset = see_line;
            token_arr[see_token] = now_token;
            see_token++;
            see++;
        }

        see++;
    }

    token end_token;
    end_token.type = END_LEXER;

    token_arr[see_token] = end_token;
    result.past_token = see_token;
    return result;
}