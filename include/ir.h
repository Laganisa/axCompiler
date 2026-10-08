#ifndef _COMPIL_IR_H_
#define _COMPIL_IR_H_

#include "types.h"
#include "call.h"
#include "hash.h"
#include "parser.h"

#define MAX_IR_SIZE 1024

enum IR_REG_TYPE
{
    IR_NULL_TYPE = 0, // 널 타입
    IR_REG_TYPE = 1,  // 임시 레지스터 번호
    IR_IMM_TYPE,      // 즉시값
    IR_VAR_TYPE,      // 변수의 심볼 번호
    IR_MEM_TYPE,      // 메모리 주소 또는 메모리 위치
    IR_LABEL_TYPE,    // 분기 대상 레이블 번호
};

typedef struct reg_op
{
    uint64_t value;
    uint8_t type;

} reg_op;

// 중간 코드
typedef struct ir
{
    uint8_t op; // 연산 타입
    struct reg_op dst;
    struct reg_op src1;
    struct reg_op src2;
} ir;

extern ir global_ir_arr[MAX_IR_SIZE];
extern uint16_t global_ir_count;

// 중간코드 에러 확인용도
// 형식상 추가
typedef struct check_Ir
{
    uint8_t error_code; // 중간코드 에러코드
    uint16_t ir_is_end; // 함수 종료코드
    uint16_t ir_count;  // 생성된 중간 코드 개수

} check_Ir;

enum IR_OP_TYPE
{
    IR_NOP = 0, // 빈 연산 (최적화로 삭제됨)

    // 데이터 이동 및 메모리 접근 (Load-Store 아키텍처 반영)
    IR_MOV,   // 레지스터 간 값 이동 (dest = src1)
    IR_LOAD,  // 스택/메모리에서 레지스터로 로드 (LDR 대응)
    IR_STORE, // 레지스터에서 스택/메모리로 저장 (STR 대응)

    // 산술 연산 (AArch64 ADD, SUB, MUL, SDIV 대응)
    IR_ADD, // 덧셈 (dest = src1 + src2)
    IR_SUB, // 뺄셈 (dest = src1 - src2)
    IR_MUL, // 곱셈 (dest = src1 * src2)
    IR_DIV, // 나눗셈 (dest = src1 / src2)

    // 비트 및 논리 연산 (AND, ORR, EOR 대응)
    IR_AND, // 비트 AND
    IR_OR,  // 비트 OR
    IR_XOR, // 비트 XOR (EOR)

    // 비교 및 제어 흐름 (AArch64 CMP 및 조건부 분기 B.xx 대응)
    IR_CMP,   // 비교 연산 (src1과 src2 비교 후 플래그 세팅)
    IR_LABEL, // 라벨 정의점 부착 (dest = 라벨 ID)
    IR_JMP,   // 무조건 점프 (B 라벨)
    IR_JE,    // 같으면 점프 (B.EQ)
    IR_JNE,   // 다르면 점프 (B.NE)
    IR_JL,    // 작으면 점프 (B.LT)
    IR_JG,    // 크면 점프 (B.GT)

    // 함수 호출 및 반환 (BL, RET 대응)
    IR_CALL, // 함수 호출
    IR_RET   // 함수 반환
};

// 중간 코드 생성 에러
enum IR_ERROR_CODE
{
    // 아직 작성하지 않음
    IR_OK = 1, // ACK
    IR_YET,    // 아직 작성되지 않은 기능 사용
};

check_Ir ir_gen(ast_node *node_arr, uint16_t node_number);

reg_op trav(ir *ir_arr, ast_node *node_arr, uint16_t node_index);

reg_op trav_expr(ir *ir_arr, ast_node *node_arr, uint16_t node_index);

#endif