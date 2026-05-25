.section .text.start
.global _start

_start:
    bl compiler_main
    mov w0, w0
    bl axlib_exit

1:
    b 1b
