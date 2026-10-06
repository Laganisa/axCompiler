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

// 심블 생성 에러
enum SYMBOL_ERROR_CODE
{
    SYMBOL_OK = 1, // ACK

    SYMBOL_MAX_SCP = MAX_SCOPE_SIZE, // 최대 스코프 사이즈

    SYMBOL_CRATE_ERR,  // 심블 생성 오류
    SYMBOL_SEARCH_ERR, // 심블 탐색 오류
};

// 스코프를 만드는 함수
scope *scope_crete(ScopeType val);
scope *scope_delate(scope *scp);

/*
    변수명이 선언되었을 때 변수 명을 현재 스코프에 넣는 로직
*/
uint8_t symbol_crate(scope *scp, char *name);

/*
    변수명이 선언 된 상태이고 표현식에서 쓰일때 스코프에서 찾는 로직
*/
uint8_t symbol_search(scope *scp, char *name);

/*
    변수명을 스코프에서 지우는 로직
*/
void symbol_delate(scope *scp, char *name);

#endif