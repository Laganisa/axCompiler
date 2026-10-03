#include "main.h"
#include "defs.h"
#include "call.h"

#include "lexer.h"
#include "parser.h"
#include "symbol.h"
#include "ir.h"
#include "string.h"
#include "memory.h"

static void foo(void)
{
    file_creat("test", "0777", 4096);
}

void compiler_main(void)
{
    write(STDIO, (const int8_t *)"%s", (const int64_t *)"compiler enter\n");

    // foo 함수를 실행해서 컴파일할 파일을 생성
    foo();

    uint8_t sign_error = 0;
    uint8_t is_eof = 0;
    uint8_t input_error = 0;

    // 토큰을 저장하는 배열
    token token_arr[MAX_TOKEN_ARR];
    axlib_memset(token_arr, 0, sizeof(token_arr));

    uint16_t now_token = 0;

    for (int i = 0; i < MAX_SOURCE_LINE; i++)
    {
        int8_t line[65];
        axlib_memset(line, 0, sizeof(line));

        long read_status = read(STDIO, (char *)line, sizeof(line));
        if (read_status == 0)
        {
            is_eof = 1;
            break;
        }
        if (read_status < 0)
        {
            sign_error = 1;
            input_error = 1;
            break;
        }

        if (axlib_strlen((const char *)line) > 63)
        {
            sign_error = 1;
            break;
        }

        check_L receive_L;

        receive_L = lexer(token_arr, line, i, now_token);
        now_token = receive_L.past_token;

        /*
            만약 함수 종료 시그널이 들어오지 않았으면
            파셔을 작동시키기로
            ! 수정하기
        */
        if (1)
        {
            // 배열 초기화
            // 추상 구문 트리의 노드들을 담은 배열
            ast_node node_arr[MAX_TOKEN_ARR];
            axlib_memset(node_arr, 0, sizeof(node_arr));

            // 추상구문트리 만들기
            check_P receive_P;
            // 심볼 분석하기

            // 코드젠 함수 부르기
        }
    }

    if (is_eof == 0 && sign_error == 0)
    {
        int8_t extra_line[2];
        axlib_memset(extra_line, 0, sizeof(extra_line));
        long read_status = read(STDIO, (char *)extra_line, sizeof(extra_line));
        if (read_status == 0)
        {
            is_eof = 1;
        }
        else
        {
            sign_error = 1;
            input_error = read_status < 0;
        }
    }

    if (is_eof == 0)
    {
        sign_error = 1;
    }

    // 수정하기
    if (sign_error != 0)
    {
        const int8_t *message = input_error != 0
                                    ? (const int8_t *)"compiler error: source read failed\n"
                                    : (const int8_t *)"compiler error: source line or file limit exceeded\n";
        write(STDIO, (const int8_t *)"%s", (const int64_t *)message);
    }
}