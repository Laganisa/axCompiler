#include "main.h"
#include "defs.h"

#include "lexer.h"
#include "parser.h"
#include "symbol.h"
#include "ir.h"

// [수정] 파일 데이터의 진짜 물리 주소(src_ptr)를 받아오도록 설계
void compiler_main(int8_t *src_ptr)
{
    compiler_write("compiler enter\n");
    uint8_t sign_error = 0;
    uint16_t lines_num = 0;
    
    uint8_t is_eof = 0; 

    // 토큰을 저장하는 배열
    token token_arr[MAX_TOKEN_ARR] = {0,};
    uint16_t now_token = 0;
    for (int i = 0; i < MAX_SOURCE_LINE; i++)
    {
        int8_t line[64] = {0, }; 

        // 2. [핵심] src_ptr에서 딱 한 줄(64글자 제한) 또는 \n 만날 때까지 line으로 복사하는 로직이 필요해!
        // (여기에 파일 읽기 또는 메모리 카피 로직이 들어가야 함)
        // ex: src_ptr = read_one_line(src_ptr, line);

        if (line[0] == '\0')
        {
            is_eof = 1; 
            break;
        }

        lines_num++;

        check_L receive;

        receive = lexer(&token_arr, &line, i, now_token);
        now_token = receive.past_token;
        
        /*
            만약 함수 종료 시그널이 들어왔으면
            파셔을 작동시키기
        */
        if (1)
        {
            // 배열 초기화
            token token_arr[MAX_TOKEN_ARR] = {0,};
            // 추상구문트리 만들기

            // 추상구문트리를 만든다음에

            // 코드젠 함수 부르기
        }
    }

    if (is_eof == 0)
    {
        // 컴파일 할 파일 크기 에러
        sign_error = 1; 
    }
}