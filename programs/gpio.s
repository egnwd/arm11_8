ldr r13, =0x10000             ; init stack pointer

ldr r4, =0x20200000           ; load base GPIO Address
ldr r5, =0x1240               ; bits to set 02, 03, 04 as outputs
str r5, [r4]                  ; set pins 02, 03, 04 as OUTPUT and pin 09 to INPUT

mov r5, #0x1c                 ; bits [2..4] = 1
mov r0, #1                    ; previous state = ON

mov r6, #0                    ; count = 0 (lsl 2, to align bits)
counterLoop:                  ; begin counter loop

str r5, [r4, #0x28]           ; clear all pins

str r6, [r4, #0x1c]           ; set pins

add r6, r6, #4                ; count++ (lsl 2, to align bits)

; call waitForObstruction
add r14, r15, #8              ; r14 = PC + 8 (just after branch)
sub r13, r13, #4              ; SP = SP - 4
str r14, [r13]                ; push return address
b fn_waitForObstruction       ; call delay

cmp r6, #32                   ; compare counter
blt counterLoop               ; if >= 32 break

andeq r0, r0, r0              ; halt

fn_waitForObstruction:

ldr r1, [r4, #0x34]           ; get inputs from pins
mov r1, r1, lsr #9            ; shift bit 9 down to bit 0
and r1, r1, #1                ; r1 = current state
eor r1, r1, #1                ; r1 = !r1
tst r1, r0                    ; !current state && previous state
eor r0, r1, #1                ; previous state = current state
bne fn_waitForObstruction     ; return to count if new obstruction

ldr r14, [r13]                ; pop return address
add r13, r13, #4              ; SP = SP + 4
mov r15, r14                  ; return
