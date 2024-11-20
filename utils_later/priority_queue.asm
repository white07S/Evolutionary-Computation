; priority_queue.asm
; Implementation of a priority queue using a max-heap in x86-64 assembly (NASM syntax)
; This code is designed to be called from C.

section .data
    heap_capacity dq 0x64        ; Default heap capacity (100 in decimal)
    newline       db 0x0A         ; Newline character
    fmt           db "%ld", 0xA, 0    ; "%ld\n", null-terminated
    fmt_overflow  db "Heap Overflow!\n", 0

section .bss
    heap_size     resq 1           ; Current size of the heap (8 bytes)
    heap          resq 100         ; Heap array with capacity for 100 qwords
    output_buffer resb 20          ; Buffer for integer to string conversion

section .text
    global insert
    global extract_max
    global print_int
    extern printf                   ; To use C's printf for output

;---------------------------------------
; void insert(int64_t value)
; Inserts a value into the priority queue
; Input: RDI = int64_t value
;---------------------------------------
insert:
    push rbp
    mov rbp, rsp

    ; Load heap_size into RAX
    mov rax, [heap_size]
    mov rcx, rax         ; rcx = index of new element

    ; Check for heap overflow
    mov rbx, [heap_capacity]
    cmp rcx, rbx
    jae insert_overflow   ; If heap_size >= heap_capacity, overflow

    ; heap[heap_size] = value
    lea rbx, [heap + rcx*8]
    mov [rbx], rdi

    ; heap_size++
    inc qword [heap_size]

    ; Percolate up
    cmp rcx, 0
    jle insert_end        ; If index <= 0, done

insert_loop:
    mov rax, rcx
    sub rax, 1
    shr rax, 1            ; rax = parent index (floor((i-1)/2))

    ; Load heap[parent]
    lea rbx, [heap + rax*8]
    mov rsi, [rbx]        ; rsi = heap[parent]

    ; Load heap[index]
    lea rbx, [heap + rcx*8]
    mov rdi, [rbx]        ; rdi = heap[index]

    ; Compare heap[parent] < heap[index]
    cmp rsi, rdi
    jge insert_end        ; If heap[parent] >= heap[index], done

    ; Swap heap[parent] and heap[index]
    push rcx              ; Save current index
    push rax              ; Save parent index

    mov rdx, rcx          ; rdx = index
    mov rcx, rax          ; rcx = parent index
    call swap

    pop rax               ; Restore parent index
    pop rcx               ; Restore current index

    ; Update index to parent index
    mov rcx, rax

    ; Continue if index > 0
    cmp rcx, 0
    jg insert_loop

insert_end:
    pop rbp
    ret

insert_overflow:
    ; Print overflow message
    lea rdi, [rel fmt_overflow]
    xor al, al
    call printf
    pop rbp
    ret

;---------------------------------------
; int64_t extract_max()
; Extracts and returns the maximum value from the priority queue
; Returns: RAX = max value
;---------------------------------------
extract_max:
    push rbp
    mov rbp, rsp

    ; Check if heap is empty
    cmp qword [heap_size], 0
    je extract_empty

    ; max = heap[0]
    mov rax, [heap]

    ; Move last element to root
    mov rcx, [heap_size]
    dec rcx
    lea rbx, [heap + rcx*8]
    mov rdx, [rbx]        ; rdx = last element
    mov [heap], rdx

    ; heap_size--
    dec qword [heap_size]

    ; Percolate down
    mov rcx, 0            ; rcx = index

extract_loop:
    ; left_child = 2 * index + 1
    mov rdx, rcx
    shl rdx, 1
    add rdx, 1

    ; Check if left_child < heap_size
    mov rsi, [heap_size]
    cmp rdx, rsi
    jge extract_end_loop

    ; Assume larger_child = left_child
    mov r8, rdx           ; r8 = larger_child

    ; right_child = 2 * index + 2
    mov r9, rcx
    shl r9, 1
    add r9, 2

    ; Check if right_child < heap_size
    cmp r9, rsi
    jge compare_children

    ; Load heap[left_child] and heap[right_child]
    lea rbx, [heap + rdx*8]
    mov rdi, [rbx]        ; rdi = heap[left_child]
    lea rbx, [heap + r9*8]
    mov rsi, [rbx]        ; rsi = heap[right_child]

    ; Compare heap[right_child] > heap[left_child]
    cmp rsi, rdi
    jle compare_children
    ; larger_child = right_child
    mov r8, r9

compare_children:
    ; Load heap[index] and heap[larger_child]
    lea rbx, [heap + rcx*8]
    mov rdi, [rbx]        ; rdi = heap[index]
    lea rbx, [heap + r8*8]
    mov rsi, [rbx]        ; rsi = heap[larger_child]

    ; If heap[index] >= heap[larger_child], break
    cmp rdi, rsi
    jge extract_end_loop

    ; Swap heap[index] and heap[larger_child]
    push rcx
    push r8
    mov rcx, rcx          ; index
    mov rdx, r8           ; larger_child
    call swap
    pop r8
    pop rcx

    ; Update index to larger_child
    mov rcx, r8
    jmp extract_loop

extract_end_loop:
    jmp extract_done

extract_empty:
    mov rax, -1           ; Return -1 if heap is empty

extract_done:
    pop rbp
    ret

;---------------------------------------
; void swap(int64_t a_index, int64_t b_index)
; Swaps heap[a_index] and heap[b_index]
; Inputs:
;   RCX = a_index
;   RDX = b_index
;---------------------------------------
swap:
    ; Load heap[a_index] into RAX
    lea rbx, [heap + rcx*8]
    mov rax, [rbx]

    ; Load heap[b_index] into RSI
    lea rbx, [heap + rdx*8]
    mov rsi, [rbx]

    ; Perform the swap
    mov [rbx], rax
    lea rbx, [heap + rcx*8]
    mov [rbx], rsi

    ret

;---------------------------------------
; void print_int(int64_t value)
; Prints the integer value to stdout using printf
; Input: RDI = int64_t value
;---------------------------------------
print_int:
    ; Call printf with format string and value
    lea rsi, [rel fmt]    ; Format string "%ld\n"
    mov rdi, fmt          ; RDI = pointer to format string
    mov rax, 0            ; Clear RAX for variadic functions
    call printf
    ret

