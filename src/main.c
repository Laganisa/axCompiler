#include "main.h"
#include "defs.h"
#include "call.h"
#include "debug.h"

#include "lexer.h"
#include "memory.h"

static uint8_t proc_src_line(
    token *token_arr,
    uint16_t *token_cnt_p,
    const int8_t src_line[64],
    uint16_t src_line_num)
{
    check_Lexer lxr_result = lexer(
        token_arr,
        src_line,
        src_line_num,
        *token_cnt_p);
    *token_cnt_p = lxr_result.lxr_past_token_cnt;

    return lxr_result.lxr_error_code == LEXER_OK;
}

static int32_t create_test_src(void)
{
    // 파일을 생성하고
    file_creat("test", "0777", 4096);

    int32_t src_writer_fd = file_open("test", 'u', 0);

    if (src_writer_fd < 0)
    {
        return src_writer_fd;
    }

    write(
        src_writer_fd,
        "%s",
        "int32 main(){return 0d0;}");

    file_close(src_writer_fd);

    int32_t src_file_fd = file_open("test", 'r', 0);

    if (src_file_fd < 0)
    {
        return src_file_fd;
    }

    write(
        STDIO,
        "%s",
        "test file created\n");
    return src_file_fd;
}

void compiler_main(void)
{
    write(
        STDIO,
        "%s",
        "compiler enter\n");

    int32_t src_file_fd = create_test_src();

    if (src_file_fd < 0)
    {
        write(
            STDIO,
            "%s",
            "compiler error: src file open failed\n");
        return;
    }

    uint8_t has_error = 0;
    uint8_t is_eof = 0;
    uint8_t is_input_error = 0;
    uint8_t is_lexer_error = 0;

    token token_arr[MAX_TOKEN_ARR];
    axlib_memset(token_arr, 0, sizeof(token_arr));

    uint16_t token_cnt = 0;
    uint16_t src_line_num = 0;
    uint16_t src_line_len = 0;
    uint8_t skip_line_feed = 0;
    int8_t src_line_buf[64] = {0};
    int8_t src_chunk[64];

    while (has_error == 0 && is_eof == 0)
    {
        int32_t read_result = axlib_read(
            src_file_fd,
            src_chunk,
            sizeof(src_chunk),
            0);

        if (read_result < 0)
        {
            has_error = 1;
            is_input_error = 1;
            break;
        }
        if (read_result == 0)
        {
            is_eof = 1;

            if (src_line_len != 0)
            {
                if (src_line_num >= MAX_SOURCE_LINE)
                {
                    has_error = 1;
                }
                else if (!proc_src_line(
                             token_arr,
                             &token_cnt,
                             src_line_buf,
                             src_line_num))
                {
                    has_error = 1;
                    is_lexer_error = 1;
                }
            }
            break;
        }

        if (read_result > (int32_t)sizeof(src_chunk))
        {
            has_error = 1;
            is_input_error = 1;
            break;
        }

        for (int32_t chunk_index = 0;
             chunk_index < read_result;
             chunk_index++)
        {
            int8_t src_char =
                src_chunk[chunk_index];

            // 파일 공간의 끝은 0으로 채워지므로 이를 입력 종료로 처리
            if (src_char == '\0')
            {
                is_eof = 1;
                if (src_line_len != 0)
                {
                    if (src_line_num >= MAX_SOURCE_LINE)
                    {
                        has_error = 1;
                    }
                    else if (!proc_src_line(
                                 token_arr,
                                 &token_cnt,
                                 src_line_buf,
                                 src_line_num))
                    {
                        has_error = 1;
                        is_lexer_error = 1;
                    }
                }
                break;
            }

            if (skip_line_feed != 0)
            {
                skip_line_feed = 0;
                if (src_char == '\n')
                {
                    continue;
                }
            }

            if (src_char == '\r' ||
                src_char == '\n')
            {
                if (src_line_num >= MAX_SOURCE_LINE)
                {
                    has_error = 1;
                    break;
                }

                if (!proc_src_line(
                        token_arr,
                        &token_cnt,
                        src_line_buf,
                        src_line_num))
                {
                    has_error = 1;
                    is_lexer_error = 1;
                    break;
                }

                src_line_num++;
                src_line_len = 0;
                axlib_memset(
                    src_line_buf,
                    0,
                    sizeof(src_line_buf));
                skip_line_feed = src_char == '\r';
                continue;
            }

            if (src_line_len >= 63)
            {
                has_error = 1;
                break;
            }
            src_line_buf[src_line_len] =
                src_char;
            src_line_len++;
        }
    }

    file_close(src_file_fd);

    if (is_eof == 0)
    {
        has_error = 1;
    }

    // 만약에 에러가 나왔다면
    if (has_error != 0)
    {
        // 기본 메시지
        const char *error_message_p =
            "compiler error: src line or file limit exceeded\n";

        // 만약에 입력 에러가 나왔다면
        if (is_input_error != 0)
        {
            error_message_p = "compiler error: src read failed\n";
        }
        // 만약에 렉서 에러가 났다면
        else if (is_lexer_error != 0)
        {
            error_message_p = "compiler error: invalid src token\n";
        }

        write(STDIO, "%s", error_message_p);
    }
}