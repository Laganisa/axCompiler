#include "symbol.h"
#include "hash.h"
#include "memory.h"

// 스코프 타입에 맞는 스코프를 생성
// 동적 할당을 이용함
scope *scope_crete(ScopeType val)
{
    if (val > 3)
    {
        return NULL;
    }

    scope *new_scp = (scope *)axlib_malloc(sizeof(scope));

    new_scp->type = val;
    new_scp->par = NULL;

    for (int i = 0; i < MAX_SCOPE_SIZE; i++)
    {
        new_scp->symbol_buf[i] = 0;
    }

    // 일단 초기화
    new_scp->local_offset = 0;

    return new_scp;
}

// 스코프 타입에 맞는 스코프를 삭제
scope *scope_delate(scope *scp)
{
    // 만약 NULL 이라면
    if (scp == NULL)
    {
        return NULL;
    }

    scope *parent = scp->par;

    axlib_free(scp);

    return parent;
}
/*
    변수명이 선언되었을 때 변수 명을 현재 스코프에 넣는 로직
*/
uint8_t symbol_crate(scope *scp, char *name)
{
    // 만약 NULL 이라면
    if (scp == NULL)
    {
        return NULL;
    }

    uint64_t hash_val = axlib_fnv1a_hash_64(name);

    uint8_t curr_num = scp->num;

    // 현재 값이 최대라면?
    // 심블 에러 나타내기
    if (curr_num >= MAX_SCOPE_SIZE)
    {
        return SYMBOL_CRATE_ERR;
    }

    for (int i = 0; i < scp->num; i++)
    {
        if (scp->symbol_buf[i] == hash_val)
        {
            return SYMBOL_CRATE_ERR; // 중복 선언 에러
        }
    }

    scp->symbol_buf[curr_num] = hash_val;
    scp->num++;
    return curr_num;
}

/*
    변수명이 선언 된 상태이고 표현식에서 쓰일때 스코프에서 찾는 로직
*/
uint8_t symbol_search(scope *scp, char *name)
{
    // 만약 NULL 이라면
    if (scp == NULL)
    {
        return NULL;
    }

    uint64_t hash_val = axlib_fnv1a_hash_64(name);

    // 성공 불리언 변수
    uint8_t is_succ = FALSE;
    uint8_t index = 0;

    // 탐색 준비
    scope *now_scp = scp;

    // while을 통한 탐색 후
    while (now_scp != NULL)
    {
        // 현재 있는 숫자 만큼 반복
        for (int i = 0; i < MAX_SCOPE_SIZE; i++)
        {
            // 만약 있다면?
            if (now_scp->symbol_buf[i] == hash_val)
            {
                is_succ = TRUE;
                index = i;
                break;
            }
        }

        // 탐색 후 위로 올라가기
        now_scp = now_scp->par;
    }

    // 못 찾으면 에러
    if (!is_succ)
    {
        return SYMBOL_SEARCH_ERR;
    }

    return index;
}

/*
    변수명을 스코프에서 지우는 로직
    상위 스코프까지 탐색하지 않음
*/
void symbol_delate(scope *scp, char *name)
{
    // 만약 NULL 이라면
    if (scp == NULL)
    {
        return;
    }

    uint64_t hash_val = axlib_fnv1a_hash_64(name);

    for (int i = 0; i < scp->num; i++)
    {
        // 만약 있다면?
        if (scp->symbol_buf[i] == hash_val)
        {
            scp->symbol_buf[i] = 0;

            // 배열 당기기
            for (int j = i; j < scp->num - 1; j++)
            {
                scp->symbol_buf[j] = scp->symbol_buf[j + 1];
            }

            scp->symbol_buf[scp->num - 1] = 0;
            scp->num--;
            break;
        }
    }
}