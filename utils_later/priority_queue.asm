; priority_queue.asm
; Implementation of a priority queue using a max-heap in x86-64 assembly (NASM syntax)

section .data
    heap_size dq 0          ; Current size of the heap

section .bss
    heap resq 100           ; Heap array with capacity for 100 qwords
    output_buffer resb 20   ; Buffer for integer to string conversion

section .text
    global _start

;---------------------------------------
; Function prototypes
;---------------------------------------
; _start
; insert
; extract_max
; swap
; print_int

;---------------------------------------
; Program Entry Point
;---------------------------------------
_start:
    ; Initialize heap_size to 0
    mov qword [heap_size], 0

    ; Insert values into the priority queue
    mov rdi, 5
    call insert
    mov rdi, 10
    call insert
    mov rdi, 3
    call insert
    mov rdi, 8
    call insert
    mov rdi, 2
    call insert

    ; Extract the maximum value from the priority queue
    call extract_max
    ; The result is in rax

    ; Print the extracted maximum value
    call print_int

    ; Exit the program
    mov rax, 60          ; sys_exit system call
    xor rdi, rdi         ; Exit code 0
    syscall

;---------------------------------------
; void insert(int value)
; Inserts a value into the priority queue
;---------------------------------------
insert:
    push rbp
    mov rbp, rsp

    ; rdi contains the value to insert
    ; Load heap_size into rax
    mov rax, [heap_size]
    mov rcx, rax         ; rcx = index of new element

    ; heap[heap_size] = value
    lea rbx, [heap + rax*8]
    mov [rbx], rdi

    ; heap_size++
    inc qword [heap_size]

    ; Percolate up
    cmp rcx, 0
    jle insert_end       ; If index <= 0, we're done

insert_loop:
    mov rax, rcx
    sub rax, 1
    shr rax, 1           ; rax = parent index

    ; Load heap[parent]
    lea rbx, [heap + rax*8]
    mov rsi, [rbx]       ; rsi = heap[parent]

    ; Load heap[index]
    lea rbx, [heap + rcx*8]
    mov rdi, [rbx]       ; rdi = heap[index]

    ; Compare heap[parent] < heap[index]
    cmp rsi, rdi
    jge insert_end       ; If heap[parent] >= heap[index], we're done

    ; Swap heap[parent] and heap[index]
    push rcx             ; Save current index
    push rax             ; Save parent index

    mov rdx, rcx         ; rdx = index
    mov rcx, rax         ; rcx = parent index
    call swap

    pop rax              ; Restore parent index
    pop rcx              ; Restore current index

    ; Update index to parent index
    mov rcx, rax

    ; Continue if index > 0
    cmp rcx, 0
    jg insert_loop

insert_end:
    pop rbp
    ret

;---------------------------------------
; int extract_max()
; Extracts and returns the maximum value from the priority queue
;---------------------------------------
extract_max:
    push rbp
    mov rbp, rsp

    ; Check if heap is empty
    cmp qword [heap_size], 0
    jg proceed_extract
    mov rax, -1          ; Return -1 if heap is empty
    jmp extract_end

proceed_extract:
    ; max = heap[0]
    mov rax, [heap]
    ; Move last element to root
    mov rcx, [heap_size]
    dec rcx
    lea rbx, [heap + rcx*8]
    mov rdx, [rbx]       ; rdx = last element
    mov [heap], rdx
    ; heap_size--
    dec qword [heap_size]

    ; Percolate down
    mov rcx, 0           ; rcx = index

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
    mov r8, rdx          ; r8 = larger_child

    ; right_child = 2 * index + 2
    mov r9, rcx
    shl r9, 1
    add r9, 2

    ; Check if right_child < heap_size
    cmp r9, rsi
    jge compare_children

    ; Load heap[left_child] and heap[right_child]
    lea rbx, [heap + rdx*8]
    mov rdi, [rbx]       ; rdi = heap[left_child]
    lea rbx, [heap + r9*8]
    mov rsi, [rbx]       ; rsi = heap[right_child]

    ; Compare heap[right_child] > heap[left_child]
    cmp rsi, rdi
    jle compare_children
    ; larger_child = right_child
    mov r8, r9

compare_children:
    ; Load heap[index] and heap[larger_child]
    lea rbx, [heap + rcx*8]
    mov rdi, [rbx]       ; rdi = heap[index]
    lea rbx, [heap + r8*8]
    mov rsi, [rbx]       ; rsi = heap[larger_child]

    ; If heap[index] >= heap[larger_child], break
    cmp rdi, rsi
    jge extract_end_loop

    ; Swap heap[index] and heap[larger_child]
    push rcx
    push r8
    mov rcx, rcx         ; index
    mov rdx, r8          ; larger_child
    call swap
    pop r8
    pop rcx

    ; Update index to larger_child
    mov rcx, r8
    jmp extract_loop

extract_end_loop:
    ; Return max value in rax
extract_end:
    pop rbp
    ret

;---------------------------------------
; void swap(int64_t a_index, int64_t b_index)
; Swaps heap[a_index] and heap[b_index]
; rcx = a_index
; rdx = b_index
;---------------------------------------
swap:
    ; Load heap[a_index] into rax
    lea rbx, [heap + rcx*8]
    mov rax, [rbx]
    ; Load heap[b_index] into rsi
    lea rbx, [heap + rdx*8]
    mov rsi, [rbx]
    ; Perform the swap
    mov [heap + rcx*8], rsi
    mov [heap + rdx*8], rax
    ret

;---------------------------------------
; void print_int()
; Prints the integer in rax to stdout
;---------------------------------------
print_int:
    push rbp
    mov rbp, rsp

    ; Save rax value
    mov rbx, rax         ; rbx = value to print

    ; Initialize pointer to end of buffer
    lea rdi, [output_buffer + 20]
    mov byte [rdi], 0    ; Null-terminate the string

    mov rcx, 0           ; Digit count

    cmp rbx, 0
    jne print_int_loop
    ; If value is 0, print '0'
    mov byte [rdi - 1], '0'
    lea rdi, [rdi - 1]
    mov rcx, 1
    jmp print_int_done

print_int_loop:
    ; Convert least significant digit to ASCII
    mov rdx, 0
    mov rax, rbx
    mov rsi, 10
    div rsi              ; rax = rbx / 10, rdx = rbx % 10
    add dl, '0'          ; Convert digit to ASCII
    lea rdi, [rdi - 1]   ; Move pointer back
    mov [rdi], dl
    inc rcx              ; Increment digit count
    mov rbx, rax
    cmp rbx, 0
    jne print_int_loop

print_int_done:
    ; Write the string to stdout
    mov rax, 1           ; sys_write system call
    mov rdi, 1           ; File descriptor 1 (stdout)
    mov rsi, rdi         ; rsi = pointer to string
    mov rdx, rcx         ; Length of the string
    syscall

    ; Write a newline character
    mov rax, 1           ; sys_write
    mov rdi, 1           ; stdout
    mov rsi, newline
    mov rdx, 1           ; Length = 1
    syscall

    pop rbp
    ret

section .data
    newline db 0x0A      ; Newline character
