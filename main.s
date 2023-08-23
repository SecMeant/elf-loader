    SECTION .rodata
syscall_nr: dd 60
exit_code: dd 0
format_str: db "%d\n", 0

    SECTION .data
data_size: dd 5
data: dd 1, 2, 3, -6, 4

    SECTION .text
_start:

    mov ecx, [rel data_size]
    lea rax, [rel data]
    xor edi, edi

loop:
    test ecx, ecx
    jz loop_end

    mov edx, DWORD [rax + rcx * 4 - 4]
    add edi, edx

    dec ecx
    jmp loop
loop_end:

    call print_int

    mov edi, [rel exit_code]
    mov eax, [rel syscall_nr]
    syscall

print_int:
    and rdi, 0xff
    add rdi, '0'
    or rdi, 0x0a00
    mov [rsp - 8], rdi

    mov eax, 1
    mov edi, 1
    lea rsi, [rsp - 8]
    mov edx, 2
    syscall

    ret

; List of symbol exports
global _start
