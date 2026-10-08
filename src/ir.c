#include "ir.h"

/*
    ast를 거쳐 왔다 가정하자
*/

// 분기 플레그 뽑는거
uint32_t global_draw_flag = 0;

// 레지스터 뽑는거
uint32_t global_draw_regs = 0;

static uint32_t draw_reg(void)
{
    return global_draw_regs++;
}

static uint32_t draw_flag(void)
{
    return global_draw_flag++;
}

static uint8_t op_case(uint8_t val)
{

    uint8_t ret;

    switch (val)
    {
    case PLUS_TOKEN:
        ret = IR_ADD;
        break;
    case MINU_TOKEN:
        ret = IR_SUB;
        break;
    case MULTI_TOKEN:
        ret = IR_MUL;
        break;
    case DIV_TOKEN:
        ret = IR_DIV;
        break;
    case AND_TOKEN:
        ret = IR_AND;
        break;
    case OR_TOKEN:
        ret = IR_OR;
        break;
    case XOR_TOKEN:
        ret = IR_XOR;
        break;
    default:

        break;
    }

    return ret;
}

// 노드 순회 로직
// ast의 타입에 따라 순회하는 순서가 달라진다
uint32_t trav(ir *ir_arr, ast_node *node_arr, uint16_t node_index)
{
    // 새롭게 만들 노드
    ir new_ir;

    // 탐색할 노드
    ast_node now_node = node_arr[node_index];

    // 사용할 변수들
    uint32_t ret_reg;

    uint32_t using_reg;

    // 탐색
    switch (node_arr[node_index].node_type)
    {
    case NUM_NODE:

        break;

    case FLOAT_NODE:

        break;

    case STRING_NODE:

        break;

    case VAR_NODE:

        break;

    case TYPE_NODE:

        break;

    case PTR_NODE:

        break;

    case UNARY_OP_NODE:

        break;

    case BINARY_OP_NODE:

        break;

    case STMT_NODE:

        break;

    case DECL_NODE:

        break;

    case EXPR_NODE:

        break;

    case BLOCK_NODE:

        break;

    // 대입 노드 처리
    case ASSIGN_NODE:

        new_ir.op = op_case(now_node.value.op);

        // 오른쪽 노드 탐색
        new_ir.src1 = trav(ir_arr, node_arr, now_node.right);

        new_ir.src2 = trav(ir_arr, node_arr, now_node.right);

        using_reg = draw_flag();
        new_ir.dest = using_reg;

        break;

    case COMPARE_OP_NODE:

        break;

    case LOGICAL_OP_NODE:

        break;

    case CALL_NODE:
        break;

    case INDEX_NODE:

        break;

    case MEMBER_NODE:

        break;

    case FUNCTION_NODE:

        break;

    case IF_NODE:

        break;

    case LOOP_NODE:

        break;

    case RETURN_NODE:

        break;

    case PARAM_NODE:

        break;

    case ARG_NODE:

        break;

    default:
        // 에러
        break;
    }
}

/*
    ast를 읽으며 파일에 작성하기

*/
check_Ir ir_gen(ast_node *node_arr, uint16_t node_number)
{
    // 에러 확인 용도
    check_Ir new_ir;

    // 전역 변수 초기화
    global_draw_flag = 0;

    // 중간 코드를 만들고
    // 파일에 쓰기
    // ast 노드가 없어질 때까지
    // 맨 앞의 루트 노드를 잡고 쭉 순회하면서 넣으면 될듯

    ir ir_arr[MAX_IR_SIZE];

    trav(ir_arr, node_arr, 0);

    /*
        파일생성
        파일의 생성시 크기는
        심블 분석을 통한 크기로 계산될 예정
        임시 4KB
        기존에 컴파일러에 들어왔던 이름을 받아서
        확장자만 ir 로 수정하기
        또는 그냥 입력한 이름을 넣기
    */
    char *new_ir_file_name = "test.ir";

    file_create(new_ir_file_name, "0777", 4096);

    // 파일 디스크럽터 설정하기
    int fd = file_open(new_ir_file_name, "w", 0);

    return new_ir;
}