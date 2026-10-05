#include "parser.h"
#include "lexer.h"
#include "token.h"

// 재귀를 위한 함수

/*
    문장의 어휘 분석
*/
check_Parser psr_stmt(
    ast_node *node_arr,
    token *token_arr,
    uint16_t *see_token,
    uint16_t token_number)
{
    // 리턴할 파서 상태
    check_Parser new_psr_state;

    // 배열에 쓸 새로운 노드
    ast_node new_node;

    // 재귀함수 호출에 쓰일 리턴 구조체
    check_Parser now_psr_state;

    // 종료 조건
    if (*see_token >= token_number)
    {
        return new_psr_state;
    }

    uint16_t current_type = token_arr[*see_token].token_type;

    // 분기문의 어휘 분석
    if (current_type == IF_TOKEN)
    {
        // 분기 토큰 소비
        *see_token++;

        // 괄호 확인
        if (token_arr[*see_token].token_type != SO_BRACKET_TOKEN)
        {
            // 구문 오류
            new_psr_state.error_code = NOT_SO_BRACKET;
            return new_psr_state;
        }

        // 표현식 파싱
        // mid에 들어갈 변수
        now_psr_state = psr_expr(
            node_arr,
            token_arr,
            see_token,
            token_number);

        // 재귀로 받은 노드 확인
        if (now_psr_state.error_code != PARSER_OK)
        {
            new_psr_state.error_code = now_psr_state.error_code;
            return new_psr_state;
        }

        // 괄호 확인
        if (token_arr[*see_token].token_type != SO_BRACKET_TOKEN)
        {
            // 구문 오류
            new_psr_state.error_code = NOT_SC_BRACKET;
            return new_psr_state;
        }

        // mid에 넣기
        new_node.mid = now_psr_state.pos;

        // 블록 파싱
        // left에 들어갈거
        now_psr_state = psr_block(
            node_arr,
            token_arr,
            see_token,
            token_number);

        // 재귀로 받은 노드 확인
        if (now_psr_state.error_code != PARSER_OK)
        {
            new_psr_state.error_code = now_psr_state.error_code;
            return new_psr_state;
        }

        // left에 넣기
        new_node.left = now_psr_state.pos;

        // else 어휘 분석
        // right에 들어갈 거
        if (token_arr[*see_token].token_type == ELSE_TOKEN)
        {
            *see_token++;

            // 다음이 if 토큰이면?
            if (token_arr[*see_token].token_type == IF_TOKEN)
            {
                now_psr_state = psr_stmt(
                    node_arr,
                    token_arr,
                    see_token,
                    token_number);
            }
            // 아니면 블럭 호출
            else
            {
                now_psr_state = psr_block(
                    node_arr,
                    token_arr,
                    see_token,
                    token_number);
            }

            if (now_psr_state.error_code != PARSER_OK)
            {
                new_psr_state.error_code = now_psr_state.error_code;
                return new_psr_state;
            }

            new_node.right = now_psr_state.pos;
        }

        // 마무리
        new_psr_state.error_code = PARSER_OK;
        new_node.node_type = IF_NODE;
    }

    // 반복문의 어휘 분석
    // 조건 반복문
    else if (current_type == WHILE_TOKEN)
    {
        *see_token++;

        // 괄호 확인
        if (token_arr[*see_token].token_type != SO_BRACKET_TOKEN)
        {
            // 구문 오류
            new_psr_state.error_code = NOT_SO_BRACKET;
            return new_psr_state;
        }

        // 괄호 확인
        if (token_arr[*see_token].token_type != SC_BRACKET_TOKEN)
        {
            // 구문 오류
            new_psr_state.error_code = NOT_SC_BRACKET;
            return new_psr_state;
        }

        // - 내부 블록 파싱
    }
    // 한정 반복문
    else if (current_type == FOR_TOKEN)
    {
        *see_token++;
        // - 조건/초기화/증감식 파싱

        // 괄호 확인
        if (token_arr[*see_token].token_type != SO_BRACKET_TOKEN)
        {
            // 구문 오류
            new_psr_state.error_code = NOT_SO_BRACKET;
            return new_psr_state;
        }

        // 괄호 확인
        if (token_arr[*see_token].token_type != SC_BRACKET_TOKEN)
        {
            // 구문 오류
            new_psr_state.error_code = NOT_SC_BRACKET;
            return new_psr_state;
        }
    }

    // 반환문 (return) 처리
    else if (current_type == RETURN_TOKEN)
    {
        *see_token++;

        now_psr_state = psr_expr(
            node_arr,
            token_arr,
            see_token,
            token_number);

        // 재귀로 받은 노드 확인
        if (now_psr_state.error_code != PARSER_OK)
        {
            new_psr_state.error_code = now_psr_state.error_code;
            return new_psr_state;
        }

        new_node.left = now_psr_state.pos;

        // - 세미콜론(SEMI_TOKEN) 소비 확인
    }

    // 자료형으로 시작하는 선언문 처리 (int, uint 등)
    else if (current_type >= UINT8_TOKEN && current_type <= INT64_TOKEN)
    {
        // - 자료형 토큰 소비
        // - 변수명(VAL_TOKEN) 소비 확인
        // - 대입('=')이 있다면 우측 표현식 파싱
        // - 세미콜론(SEMI_TOKEN) 소비 확인
    }

    // 중괄호 블록 처리
    else if (current_type == MO_BRACKET_TOKEN)
    {
        // - parser_block 함수 호출해서 중괄호 내부 전체 처리
    }

    // 표현식 처리
    else if (current_type == VAL_TOKEN)
    {
        // - 변수명 소비 또는 표현식 파싱으로 연결
        // - 대입 연산자(=) 뒤의 식 파싱
        // - 세미콜론(SEMI_TOKEN) 소비 확인
    }

    // ! 이거 필요한가
    *see_token++;

    // 생성된 new_node를 배열에 넣기

    return new_psr_state;
}

/*
    선언의 어휘 분석
*/
check_Parser psr_decl(ast_node *node_arr, token *token_arr, uint16_t *see_token, uint16_t token_number)
{
    check_Parser new_psr_state;

    ast_node new_node;

    // 종료 조건
    if (*see_token >= token_number)
    {
        return new_psr_state;
    }

    // 1. 첫 번째 토큰이 자료형인지 확인 (UINT8_TOKEN ~ INT64_TOKEN)
    uint16_t current_type = token_arr[*see_token].token_type;
    if (current_type >= UINT8_TOKEN && current_type <= INT64_TOKEN)
    {
        // 자료형 토큰 소비
        *see_token++;
    }
    else
    {
        // 자료형이 아니면 선언문이 아님 -> 에러 처리 또는 탈출
        return new_psr_state;
    }

    // 2. 다음 토큰이 변수 이름(VAL_TOKEN)인지 확인
    if (*see_token < token_number && token_arr[*see_token].token_type == VAL_TOKEN)
    {
        // 변수 이름 토큰 소비
        *see_token++;
    }
    else
    {
        // 에러: 변수 이름이 누락됨
        return new_psr_state;
    }

    // 3. 초기화 대입('=')이 있는지 확인 (선택 사항)
    // 만약 토큰에 ASSIGN_TOKEN이 있다면 (없다면 바로 세미콜론으로 가거나 건너뜀)
    if (*see_token < token_number && token_arr[*see_token].token_type == ASSIGN_TOKEN)
    {
        *see_token++; // '=' 소비

        // 우측 표현식 파싱 함수 호출 (표현식 파서가 처리한 뒤의 토큰 위치를 받아옴)
        // *see_token = parser_expr(..., *see_token);
        // node.right = ...
    }

    // 4. 문장의 끝을 알리는 세미콜론(SEMI_TOKEN) 확인
    if (*see_token < token_number && token_arr[*see_token].token_type == SEMI_TOKEN)
    {
        *see_token++; // ';' 소비
    }
    else
    {
        // 에러: 세미콜론 누락
    }
    return new_psr_state;
}

/*
    표현식의 어휘 분석
*/
check_Parser psr_expr(ast_node *node_arr, token *token_arr, uint16_t *see_token, uint16_t token_number)
{
    check_Parser new_psr_state;
    ast_node new_node;

    // [초안 아이디어: 항 + (연산자 + 항) 반복 구조]

    // 1. 첫 번째 항(Term) 파싱 (숫자, 변수, 혹은 괄호로 둘러싼 식 등)
    // new_node = parser_term(node_arr, token_arr, token_number, *see_token);

    // 2. 이항 연산자(+, -, *, / 등)가 이어지는 동안 반복
    while (*see_token < token_number)
    {
        uint16_t op_type = token_arr[*see_token].token_type;

        // 현재 토큰이 산술 연산자(PLUS_TOKEN, MINU_TOKEN 등)가 아니라면 표현식 종료
        if (op_type != PLUS_TOKEN && op_type != MINU_TOKEN &&
            op_type != MULTI_TOKEN && op_type != DIV_TOKEN)
        {
            break;
        }

        // 연산자 토큰 소비
        *see_token++;

        // 3. 우측 항 파싱
        // new_node = parser_term(node_arr, token_arr, token_number, *see_token);

        // (참고) 이 시점에서 좌측 노드와 우측 노드를 하나의 이항 연산 AST 노드로 묶어줍니다.
    }

    return new_psr_state;
}

/*
    블럭의 어휘 분석
*/
check_Parser psr_block(ast_node *node_arr, token *token_arr, uint16_t *see_token, uint16_t token_number)
{
    check_Parser new_psr_state;

    ast_node new_node;

    if (*see_token >= token_number)
    {
        return new_psr_state;
    }

    // 1. 열린 중괄호 '{' (MO_BRACKET_TOKEN) 확인 및 소비
    if (*see_token < token_number && token_arr[*see_token].token_type == MO_BRACKET_TOKEN)
    {
        *see_token++;
    }
    else
    {
        // 에러: '{'가 없음
        return new_psr_state;
    }

    // 2. 닫힌 중괄호 '}' (MC_BRACKET_TOKEN)가 나올 때까지 내부 문장 반복 파싱
    while (*see_token < token_number)
    {
        // 만약 현재 토큰이 닫힌 중괄호라면 블록 탈출
        if (token_arr[*see_token].token_type == MC_BRACKET_TOKEN)
        {
            *see_token++; // '}' 소비
            break;
        }

        // 내부 문장 파싱 함수 호출 (반환받은 다음 토큰 위치로 갱신)
        // *see_token = parser_stmt(node_arr, token_arr, token_number, *see_token);
    }

    return new_psr_state;
}

/*
    추상 노드에 넣기 위해 토큰 배열과 그 배열의 개수를 입력 받음
*/
check_Parser parser(
    ast_node *node_arr,
    token *token_arr,
    uint16_t token_number)
{

    /*
        파서 로직
        현재는 재귀 LL(1) 파서임
        ? 나중에 LR 파서로 바꿀 의항이 있음
    */

    // 파서 쪽에서 에러나면 확인하는 구조체
    check_Parser new_psr_state;

    // 내가 보고 있는 토큰
    uint16_t *see_token = 0;

    // 받은 배열을 전부 순회
    while (*see_token < token_number)
    {
        new_psr_state = psr_stmt(
            node_arr,
            token_arr,
            *see_token, token_number);

        // 에러 확인
        if (new_psr_state.error_code != PARSER_OK)
        {
            break;
        }
    }

    // 재귀로 받은 결과값이 ok 사인이 아니라면
    // 에러코드를 그대로 넣기
    // new_node.error_code = 0

    return new_psr_state;
}