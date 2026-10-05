#ifndef _COMPIL_SYMBOL_H_
#define _COMPIL_SYMBOL_H_

#include "call.h"
#include "types.h"
#include "hash.h"

#define MAX_SCOPE_SIZE 10

typedef enum
{
    SCOPE_GLOBAL,   // 전역 스코프 (파일 전체)
    SCOPE_FUNCTION, // 함수 스코프 (함수 파라미터 및 지역 변수)
    SCOPE_BLOCK     // 일반 블록 스코프 (if문, while문, 중괄호 {} 블록 등)
} ScopeType;

typedef struct scope
{
    ScopeType type;       // 스코프 타입(예 전역인지 지역(함수 내부, 블럭 내부)인지 확인 용도)
    uint8_t local_offset; // 스코프 내부 지역 변수들의 오프셋 계산 용도
    uint8_t num;          // 현재 이 스코프에 얼마나 들어있는지

    struct scope *par;                   // 부모 스코프
    uint64_t symbol_buf[MAX_SCOPE_SIZE]; // 버퍼

} scope;

// 스코프를 만드는 함수
scope *scope_crete(ScopeType);
void scope_delate(scope *scp);

/*
    현재 열린 스코프랑 char *가 들어오면 해시를 통한 탐색
    해시를 배열에 넣고 그 인덱스를 반환
*/
uint8_t symbol_table(scope *scp, char *name);

#endif