/*
    ir 배열을 파일로 출력하는 로직
*/

void ir_insert()
{
    char *new_ir_file_name = "test.ir";

    file_create(new_ir_file_name, "0777", 4096);

    // 파일 디스크럽터 설정하기
    int fd = file_open(new_ir_file_name, "w", 0);

    while (1)
    {
    }
}