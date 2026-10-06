#include "ir.h"

/*
    ast를 거쳐 왔다 가정하자
*/

// 노드 순회 로직
// ast의 타입에 따라 순회하는 순서가 달라진다
void trav(int ir_fd, ast_node *node_arr)
{
    // 파일에 쓸 코드
    char *ir_code;

    // 탐색

    // 파일에 쓰기
    write(ir_fd, "%s", ir_code);
}

/*
    ast를 읽으며 파일에 작성하기

*/
check_Ir ir(ast_node *node_arr, uint16_t node_number)
{
    // 에러 확인 용도
    check_Ir new_ir;

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
    int fd = file_open(new_ir_file_name, "r", 0);

    // 중간 코드를 만들고
    // 파일에 쓰기
    // ast 노드가 없어질 때까지
    // 맨 앞의 루트 노드를 잡고 쭉 순회하면서 넣으면 될듯
    trav(fd, node_arr);

    return new_ir;
}