#ifndef __COMPIL_LEXER_H__
#define __COMPIL_LEXER_H__

#include "kernel.h"
#include "defs.h"
#include "token.h"

// 렉서 체크 튜플
typedef struct check_Lexer
{
    uint8_t lxr_error_code;
    uint16_t lxr_is_end : 1;
    uint16_t lxr_past_token_cnt : 15;
} check_Lexer;

check_Lexer lexer(
    token *lxr_token_arr,
    const int8_t lexer_src_line[64],
    uint16_t lxr_src_line_num,
    uint16_t lxr_token_cnt);

#endif