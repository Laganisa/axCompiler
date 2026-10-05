#include "symbol.h"
#include "hash.h"

// 스코프 타입에 맞는 스코프를 생성
scope *scope_crete(ScopeType val)
{
    scope *new_scp;
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
void scope_delate(scope *scp)
{
    // 부모로 스코프를 옮기기
    scp = scp->par;
}

/*
    char *가 들어오면 해시를 통한 탐색
    해시를 배열에 넣고 그 인덱스를 반환
*/
uint8_t symbol_table(scope *scp, char name)
{
    // 해시로 변환
    uint64_t hash_val = axlib_fnv1a_hash_64(name);

    uint8_t curr_num = scp->num;

    // 성공 불리언 변수
    uint8_t is_succ = FALSE;

    // 탐색 준비
    scope *now_scp = scp;

    // while을 통한 탐색 후
    while (now_scp->par != NULL)
    {
        // 현재 있는 숫자 만큼 반복
        for (int i = 0; i < curr_num; i++)
        {
            // 만약 있다면?
            if (now_scp->symbol_buf[i] = hash_val)
            {
                is_succ = TRUE;
                break;
            }
        }

        // 탐색 후 위로 올라가기
        now_scp = now_scp->par;
    }

    // 없으면 현재 스코프 버퍼에 넣고
    if (!is_succ)
    {
        scp->symbol_buf[scp->num] = hash_val;
        scp->num++; // 값을 증가시키고

        return;
    }

    // 있으면 그 인덱스 반환

    return;
}