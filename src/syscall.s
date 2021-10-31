.intel_syntax noprefix
.text

.global syscall

syscall:
  mov rax,0
  mov r10,rcx
  syscall
  ret
