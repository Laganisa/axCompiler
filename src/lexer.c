#include "lexer.h"
#include "defs.h"


check_L lexer(token *token_arr, int8_t see ,int8_t arr[64])
{
    check_L result;
    // 입력 줄이 들어오면 토큰으로 분리하는 로직
    // 스위치 케이스 문으로 바꿔서 만들기
    
    // 상태공간 만들기
    while (see < 64)
    {
        // 자료형의 인식
        if (arr[see] == "u" || arr[see] == "i")
        {
            see++;
        }

        // 명칭의 인식
        if (arr[see] == "")
        {
            //
        }
        // 정수의 인식
    
        // 실수의 인식

        // 문자 인식

        // 주석의 인식

        // 구분자 및 괄호의 인식
    }
    return result;
}