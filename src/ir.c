#include "ir.h"

/*
    ast를 거쳐 왔다 가정하자
*/

// 분기 플레그 뽑는거
uint32_t global_draw_flag = 0;

// 레지스터 뽑는거
uint32_t global_draw_regs = 0;

ir global_ir_arr[MAX_IR_SIZE];
uint16_t global_ir_count = 0;

uint16_t global_node_number = 0;
uint8_t global_ir_error = IR_OK;

#pragma region 정적 함수

static uint32_t draw_reg(void)
{
    return global_draw_regs++;
}

static uint32_t draw_flag(void)
{
    return global_draw_flag++;
}

// 값을 넣으면 중간 코드 연산에 맞는 이넘을 리턴
static uint8_t op_case(uint8_t val)
{
    uint8_t ret = IR_NOP;

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

// 새 명령을 배열 끝에 넣기
static void ir_store(ir *ir_arr, ir new_ir)
{
    if (global_ir_error != IR_OK)
    {
        return;
    }

    if (ir_arr == 0 || global_ir_count >= MAX_IR_SIZE)
    {
        global_ir_error = IR_YET;
        return;
    }

    ir_arr[global_ir_count] = new_ir;
    global_ir_count++;
}

// 주소 대신 분기 대상 번호를 넣기
static void ir_jump(ir *ir_arr, uint8_t op, uint32_t flag)
{
    ir new_ir = {0};
    new_ir.op = op;
    new_ir.dst.value = flag;
    new_ir.dst.type = IR_LABEL_TYPE;
    ir_store(ir_arr, new_ir);
}

static void ir_label(ir *ir_arr, uint32_t flag)
{
    ir new_ir = {0};
    new_ir.op = IR_LABEL;
    new_ir.dst.value = flag;
    new_ir.dst.type = IR_LABEL_TYPE;
    ir_store(ir_arr, new_ir);
}

static void trav_cond(
    ir *ir_arr,
    ast_node *node_arr,
    uint16_t node_index,
    uint32_t true_flag,
    uint32_t false_flag);

// 조건식을 0 또는 1 값으로 만드는 코드
static reg_op trav_bool(ir *ir_arr, ast_node *node_arr, uint16_t node_index)
{
    reg_op ret = {0, 0};
    uint32_t true_flag = draw_flag();
    uint32_t false_flag = draw_flag();
    uint32_t end_flag = draw_flag();

    ret.value = draw_reg();
    ret.type = IR_REG_TYPE;

    trav_cond(
        ir_arr,
        node_arr,
        node_index,
        true_flag,
        false_flag);

    ir_label(ir_arr, true_flag);

    ir new_ir = {0};
    new_ir.op = IR_MOV;
    new_ir.dst = ret;
    new_ir.src1.value = 1;
    new_ir.src1.type = IR_IMM_TYPE;
    ir_store(ir_arr, new_ir);
    ir_jump(ir_arr, IR_JMP, end_flag);

    ir_label(ir_arr, false_flag);

    new_ir = (ir){0};
    new_ir.op = IR_MOV;
    new_ir.dst = ret;
    new_ir.src1.value = 0;
    new_ir.src1.type = IR_IMM_TYPE;
    ir_store(ir_arr, new_ir);

    ir_label(ir_arr, end_flag);
    return ret;
}

// 비교와 논리 연산은 참/거짓 분기로 바꿔서 처리한다
static void trav_cond(
    ir *ir_arr,
    ast_node *node_arr,
    uint16_t node_index,
    uint32_t true_flag,
    uint32_t false_flag)
{
    if (global_ir_error != IR_OK)
    {
        return;
    }

    if (node_arr == 0 || node_index >= global_node_number)
    {
        global_ir_error = IR_YET;
        return;
    }

    ast_node now_node = node_arr[node_index];

    if (now_node.node_type == LOGICAL_OP_NODE)
    {
        uint32_t next_flag = draw_flag();

        if (now_node.value.op == LOGICAL_AND_TOKEN)
        {
            trav_cond(
                ir_arr,
                node_arr,
                now_node.left,
                next_flag,
                false_flag);
            ir_label(ir_arr, next_flag);
            trav_cond(
                ir_arr,
                node_arr,
                now_node.right,
                true_flag,
                false_flag);
        }
        else if (now_node.value.op == LOGICAL_OR_TOKEN)
        {
            trav_cond(
                ir_arr,
                node_arr,
                now_node.left,
                true_flag,
                next_flag);
            ir_label(ir_arr, next_flag);
            trav_cond(
                ir_arr,
                node_arr,
                now_node.right,
                true_flag,
                false_flag);
        }
        else
        {
            global_ir_error = IR_YET;
        }
        return;
    }

    if (now_node.node_type == UNARY_OP_NODE &&
        now_node.value.op == NOT_TOKEN)
    {
        trav_cond(
            ir_arr,
            node_arr,
            now_node.right,
            false_flag,
            true_flag);
        return;
    }

    if (now_node.node_type == COMPARE_OP_NODE)
    {
        reg_op src1 = trav_expr(ir_arr, node_arr, now_node.left);
        reg_op src2 = trav_expr(ir_arr, node_arr, now_node.right);
        if (global_ir_error != IR_OK)
        {
            return;
        }

        ir new_ir = {0};
        new_ir.op = IR_CMP;
        new_ir.src1 = src1;
        new_ir.src2 = src2;
        ir_store(ir_arr, new_ir);

        switch (now_node.value.op)
        {
        case EQUAL_TOKEN:
            ir_jump(ir_arr, IR_JE, true_flag);
            break;
        case NOT_EQUAL_TOKEN:
            ir_jump(ir_arr, IR_JNE, true_flag);
            break;
        case LESS_TOKEN:
            ir_jump(ir_arr, IR_JL, true_flag);
            break;
        case LESS_EQUAL_TOKEN:
            ir_jump(ir_arr, IR_JL, true_flag);
            ir_jump(ir_arr, IR_JE, true_flag);
            break;
        case GREATER_TOKEN:
            ir_jump(ir_arr, IR_JG, true_flag);
            break;
        case GREATER_EQUAL_TOKEN:
            ir_jump(ir_arr, IR_JG, true_flag);
            ir_jump(ir_arr, IR_JE, true_flag);
            break;
        default:
            global_ir_error = IR_YET;
            return;
        }

        ir_jump(ir_arr, IR_JMP, false_flag);
        return;
    }

    // 비교식이 아니면 0이 아닌 값을 참으로 본다
    reg_op src1 = trav_expr(ir_arr, node_arr, node_index);
    if (global_ir_error != IR_OK)
    {
        return;
    }

    ir new_ir = {0};
    new_ir.op = IR_CMP;
    new_ir.src1 = src1;
    new_ir.src2.value = 0;
    new_ir.src2.type = IR_IMM_TYPE;
    ir_store(ir_arr, new_ir);
    ir_jump(ir_arr, IR_JNE, true_flag);
    ir_jump(ir_arr, IR_JMP, false_flag);
}

#pragma endregion

// 노드 순회 로직
// ast의 타입에 따라 순회하는 순서가 달라진다
reg_op trav(ir *ir_arr, ast_node *node_arr, uint16_t node_index)
{
    // trav를 직접 부를 때는 파서가 사용하는 최대 노드 수를 경계로 잡는다
    global_draw_flag = 0;
    global_draw_regs = 0;
    global_ir_count = 0;
    global_node_number = MAX_PARSER_BUF;
    global_ir_error = IR_OK;

    return trav_expr(ir_arr, node_arr, node_index);
}

reg_op trav_expr(ir *ir_arr, ast_node *node_arr, uint16_t node_index)
{
    reg_op ret = {0, 0};

    if (global_ir_error != IR_OK)
    {
        return ret;
    }

    if (ir_arr == 0 || node_arr == 0 ||
        node_index >= global_node_number)
    {
        global_ir_error = IR_YET;
        return ret;
    }

    ast_node now_node = node_arr[node_index];

    switch (now_node.node_type)
    {
    case NUM_NODE:
        // 숫자는 명령을 만들지 않고 즉시값으로 돌려준다
        ret.value = now_node.value.num;
        ret.type = IR_IMM_TYPE;
        break;

    case FLOAT_NODE:
        // 현재 AST에는 실수 토큰의 값이 아니라 토큰 위치만 들어있다
        global_ir_error = IR_YET;
        break;

    case VAR_NODE:
        ret.value = now_node.value.sym;
        ret.type = IR_VAR_TYPE;
        break;

    case TYPE_NODE:
    case PARAM_NODE:
        // 자료형과 매개변수 선언은 실행 명령이 아니다
        break;

    case PTR_NODE:
    case STRING_NODE:
    case CALL_NODE:
    case INDEX_NODE:
    case MEMBER_NODE:
        // 포인터와 메모리 접근, 함수 호출은 아직 IR 명령이 없다
        global_ir_error = IR_YET;
        break;

    case UNARY_OP_NODE:
        if (now_node.value.op == PLUS_TOKEN)
        {
            ret = trav_expr(ir_arr, node_arr, now_node.right);
        }
        else if (now_node.value.op == MINU_TOKEN)
        {
            reg_op src1 = {0, 0};
            src1.value = 0;
            src1.type = IR_IMM_TYPE;
            reg_op src2 = trav_expr(ir_arr, node_arr, now_node.right);
            if (global_ir_error != IR_OK)
            {
                break;
            }

            ret.value = draw_reg();
            ret.type = IR_REG_TYPE;

            ir new_ir = {0};
            new_ir.op = IR_SUB;
            new_ir.dst = ret;
            new_ir.src1 = src1;
            new_ir.src2 = src2;
            ir_store(ir_arr, new_ir);
        }
        else if (now_node.value.op == NOT_TOKEN)
        {
            ret = trav_bool(ir_arr, node_arr, node_index);
        }
        else
        {
            global_ir_error = IR_YET;
        }
        break;

    case BINARY_OP_NODE:
    {
        uint8_t op = op_case(now_node.value.op);
        if (op == IR_NOP)
        {
            global_ir_error = IR_YET;
            break;
        }

        reg_op src1 = trav_expr(ir_arr, node_arr, now_node.left);
        reg_op src2 = trav_expr(ir_arr, node_arr, now_node.right);
        if (global_ir_error != IR_OK)
        {
            break;
        }

        ret.value = draw_reg();
        ret.type = IR_REG_TYPE;

        ir new_ir = {0};
        new_ir.op = op;
        new_ir.dst = ret;
        new_ir.src1 = src1;
        new_ir.src2 = src2;
        ir_store(ir_arr, new_ir);
        break;
    }

    case STMT_NODE:
        if (now_node.left != PSR_NO_NODE)
        {
            ret = trav_expr(ir_arr, node_arr, now_node.left);
        }
        break;

    case DECL_NODE:
        if (now_node.right != PSR_NO_NODE)
        {
            if (now_node.mid >= global_node_number ||
                node_arr[now_node.mid].node_type != VAR_NODE)
            {
                global_ir_error = IR_YET;
                break;
            }

            reg_op src1 = trav_expr(ir_arr, node_arr, now_node.right);
            if (global_ir_error != IR_OK)
            {
                break;
            }

            ir new_ir = {0};
            new_ir.op = IR_MOV;
            new_ir.dst.value = node_arr[now_node.mid].value.sym;
            new_ir.dst.type = IR_VAR_TYPE;
            new_ir.src1 = src1;
            ir_store(ir_arr, new_ir);

            ret = new_ir.dst;
        }
        break;

    case EXPR_NODE:
        if (now_node.left != PSR_NO_NODE)
        {
            ret = trav_expr(ir_arr, node_arr, now_node.left);
        }
        break;

    case BLOCK_NODE:
    {
        uint16_t stmt_pos = now_node.left;
        uint16_t step = 0;

        while (stmt_pos != PSR_NO_NODE &&
               global_ir_error == IR_OK &&
               step < global_node_number)
        {
            if (stmt_pos >= global_node_number ||
                node_arr[stmt_pos].node_type != STMT_NODE)
            {
                global_ir_error = IR_YET;
                break;
            }

            ast_node stmt_node = node_arr[stmt_pos];
            trav_expr(ir_arr, node_arr, stmt_pos);
            stmt_pos = stmt_node.right;
            step++;
        }

        if (step >= global_node_number && stmt_pos != PSR_NO_NODE)
        {
            global_ir_error = IR_YET;
        }
        break;
    }

    case ASSIGN_NODE:
    {
        if (now_node.value.op != ASSIGN_TOKEN ||
            now_node.left >= global_node_number ||
            node_arr[now_node.left].node_type != VAR_NODE)
        {
            global_ir_error = IR_YET;
            break;
        }

        reg_op src1 = trav_expr(ir_arr, node_arr, now_node.right);
        if (global_ir_error != IR_OK)
        {
            break;
        }

        ret.value = node_arr[now_node.left].value.sym;
        ret.type = IR_VAR_TYPE;

        ir new_ir = {0};
        new_ir.op = IR_MOV;
        new_ir.dst = ret;
        new_ir.src1 = src1;
        ir_store(ir_arr, new_ir);
        break;
    }

    case COMPARE_OP_NODE:
    case LOGICAL_OP_NODE:
        ret = trav_bool(ir_arr, node_arr, node_index);
        break;

    case IF_NODE:
    {
        uint32_t body_flag = draw_flag();
        uint32_t else_flag = draw_flag();
        uint32_t end_flag = now_node.right == PSR_NO_NODE
                                ? else_flag
                                : draw_flag();

        trav_cond(
            ir_arr,
            node_arr,
            now_node.mid,
            body_flag,
            else_flag);
        ir_label(ir_arr, body_flag);
        trav_expr(ir_arr, node_arr, now_node.left);

        if (now_node.right != PSR_NO_NODE)
        {
            ir_jump(ir_arr, IR_JMP, end_flag);
            ir_label(ir_arr, else_flag);
            trav_expr(ir_arr, node_arr, now_node.right);
        }

        ir_label(ir_arr, end_flag);
        break;
    }

    case LOOP_NODE:
    {
        uint32_t start_flag = draw_flag();
        uint32_t body_flag = draw_flag();
        uint32_t end_flag = draw_flag();

        if (now_node.value.op == WHILE_TOKEN)
        {
            ir_label(ir_arr, start_flag);
            trav_cond(
                ir_arr,
                node_arr,
                now_node.mid,
                body_flag,
                end_flag);
            ir_label(ir_arr, body_flag);
            trav_expr(ir_arr, node_arr, now_node.right);
            ir_jump(ir_arr, IR_JMP, start_flag);
            ir_label(ir_arr, end_flag);
        }
        else if (now_node.value.op == FOR_TOKEN)
        {
            if (now_node.left != PSR_NO_NODE)
            {
                trav_expr(ir_arr, node_arr, now_node.left);
            }

            if (now_node.right >= global_node_number ||
                node_arr[now_node.right].node_type != STMT_NODE)
            {
                global_ir_error = IR_YET;
                break;
            }

            ast_node step_node = node_arr[now_node.right];
            ir_label(ir_arr, start_flag);

            if (now_node.mid == PSR_NO_NODE)
            {
                ir_jump(ir_arr, IR_JMP, body_flag);
            }
            else
            {
                trav_cond(
                    ir_arr,
                    node_arr,
                    now_node.mid,
                    body_flag,
                    end_flag);
            }

            ir_label(ir_arr, body_flag);
            if (step_node.right != PSR_NO_NODE)
            {
                trav_expr(ir_arr, node_arr, step_node.right);
            }
            if (step_node.left != PSR_NO_NODE)
            {
                trav_expr(ir_arr, node_arr, step_node.left);
            }
            ir_jump(ir_arr, IR_JMP, start_flag);
            ir_label(ir_arr, end_flag);
        }
        else
        {
            global_ir_error = IR_YET;
        }
        break;
    }

    case RETURN_NODE:
    {
        ir new_ir = {0};
        new_ir.op = IR_RET;
        if (now_node.left != PSR_NO_NODE)
        {
            new_ir.src1 = trav_expr(ir_arr, node_arr, now_node.left);
        }
        ir_store(ir_arr, new_ir);
        break;
    }

    case FUNCTION_NODE:
        // 함수의 선언 정보는 건너뛰고 본문만 순회한다
        if (now_node.right != PSR_NO_NODE)
        {
            trav_expr(ir_arr, node_arr, now_node.right);
        }
        break;

    default:
        global_ir_error = IR_YET;
        break;
    }

    return ret;
}

/*
    ast를 읽으며 중간 코드 배열을 채우기
*/
check_Ir ir_gen(ast_node *node_arr, uint16_t node_number)
{
    check_Ir new_ir = {0};

    // 새 생성 때마다 레지스터와 분기 번호를 처음부터 사용한다
    global_draw_flag = 0;
    global_draw_regs = 0;
    global_ir_count = 0;
    global_node_number = node_number;
    global_ir_error = IR_OK;

    if (node_arr == 0 || node_number == 0)
    {
        new_ir.error_code = IR_YET;
        return new_ir;
    }

    // 남은 배열 칸을 비워 이전 생성 결과와 섞이지 않게 한다
    for (uint16_t ir_pos = 0; ir_pos < MAX_IR_SIZE; ir_pos++)
    {
        global_ir_arr[ir_pos] = (ir){0};
    }

    // 파서는 자식 노드를 먼저 넣으므로 마지막 노드가 루트다
    trav_expr(global_ir_arr, node_arr, (uint16_t)(node_number - 1));

    new_ir.error_code = global_ir_error;
    new_ir.ir_count = global_ir_count;

    return new_ir;
}