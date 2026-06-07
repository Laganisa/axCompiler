#include "lexer.h"
#include "string.h"

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

check_L lexer(token *token_arr, int8_t arr[64], uint8_t see_line, uint16_t see_token)
{
    // 리턴할 (어디를 읽을지, 에러코드)튜플
    check_L result;
    // 토큰 배열에 넣을 위치
    
    // 입력 줄이 들어오면 토큰으로 분리하는 로직

    uint8_t see = 0;
    while (see <= 63){
        // 토큰 배열에 넣을
        token now_token;

        // 명칭의 인식
        if (arr[see] == 0x5f || is_letter(arr[see]))
        {
            now_token.type = 1; // 명칭 타입
            
            uint8_t cnt = 0;
            uint8_t now_col = see;

            // 줄의 끝까지
            while (see <= 63)
            {

                if (arr[see] == 0x5f || is_letter(arr[see]) || is_dec_number(arr[see]))
                {
                    now_token.data.name[cnt] = arr[see];
                }else
                {
                    break;
                }
                see++;
                now_col++;
            }
            now_token.col = now_col;
            now_token.offset = see_line;
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
        else if(arr[see] == '0')
        {
            see++;
            // 이진수
            if (arr[see] == 'b')
            {
                while (see <= 63)
                {
                    if (arr[see] == '0' && arr[see] == '1')
                    {
                        // 로직
                    }
                    else
                    {
                        // 나가기
                    }
                }
            }
            // 십진수
            else if (arr[see] == 'd') 
            {
            
            }
            // 16진수
            else if (arr[see] == 'x')
            {
            
            }
            // 실수(정밀도, 배정밀도)
            else if (arr[see] == 'f' )
            {
            
            }
            else
            {
                result.error_code = PREFIX_ERROR; // 접두사 선언 에러
                break;
            }

            token_arr[see_token] = now_token;
            see_token++;
        }
        // 주석의 인식(// \n이 나올때까지 토큰을 생성하지 않기)
        else if (arr[see] == '/')
        {
            see++;
            if (arr[see] == '/')
            {
                // 주석의 처리
                break;
            }
            else
            {
                // 주석 에러
                result.error_code = ANNOTATION_ERROR;
                break;
            }
        }
        // 문자의 인식
        else if (arr[see] == 34)
        {
            // 다시 34가 나올때가지 while
        }
        // 구분자 및 괄호의 인식
        else if (arr[see] == '(')
        {
            // 열린 괄호
        }
        else if (arr[see] == ')')
        {
            // 닫힌 괄호
        }
        else if (arr[see] == '{')
        {
            // 열린 중괄호
        }
        else if (arr[see] == '}')
        {
            // 닫힌 중괄호
        }

        // 토큰 배열에 넣기

        see++;
    }

    result.past_token =     see_token;
    return result;
}