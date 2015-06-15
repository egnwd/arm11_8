ldr r13, =0x10000   ; init stack pointer
ldr r4, =0x20200000 ; load base GPIO Address
ldr r5, =0x1240     ; bits to set 02, 03, 04 as outputs
str r5, [r4]        ; set pins 02, 03, 04 as OUTPUT
                    ; set pin 09 as INPUT

mov r5, #0x1c       ; bits [2..4] = 1

loop:               ; will keep repeating

str r5, [r4, #0x28] ; clear all pins

; call flasher
add r14, r15, #8    ; r14 = PC + 8 (just after branch)
sub r13, r13, #4    ; SP = SP - 4
str r14, [r13]      ; push return address
b fn_flasher        ; call flasher

ldr r0, =0xfff000   ; set delay = 0xfff000
delay:              ; begin delay
sub r0, r0, #1
cmp r0, #0
bne delay           ; end delay

b loop              ; back to loop

andeq r0,r0,r0

fn_flasher:

ldr r1, [r4, #0x34] ; r1 = input bits
and r1, r1, #0x200  ; only bit 9 (current state)
cmp r1, #0x200      ; check current state
bne end             ; if on, set pins
str r5, [r4, #0x1c] ; set pins
end:

ldr r14, [r13]      ; pop return address
add r13, r13, #4    ; SP = SP + 4
mov r15, r14        ; return
