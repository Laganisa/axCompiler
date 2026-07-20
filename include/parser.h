#ifndef __PARSER_H__
#define __PARSER_H__

#include "kernel.h"
#include "defs.h"
#include "lexer.h"

typedef struct parser_Resu
{
    uint8_t state;
    uint16_t token_idx;
    
} parser_Resu ;

typedef struct parser_node_buf
{
    // 파서 버퍼
    uint16_t sp; // 스택 포인터
    uint16_t current_pos;

    struct token buf[6*MAX_PARSER_BUF]; // 15.3 KB
    struct parser_Resu Resu_arr[MAX_PARSER_BUF]; // 파서 재귀 스택 500개 
};

typedef struct check_P
{
    uint8_t error_code; // 에러코드
    uint16_t is_end : 1; // 함수 종료코드
    uint16_t read_token : 15; // 어디까지 토큰 배열을 읽었는지
} check_P ;

typedef struct node
{
    uint8_t type;
    
    // 위치를 가리키는 변수 
    uint16_t right;
    uint16_t mid;
    uint16_t left;

} node ;

// 함수들
check_P parser(node *node_arr, token *token_arr, uint16_t token_number);
void LL_parser();

#endif