ldr r4, =0x20200000 ; load base GPIO Address
ldr r5, =0x1240     ; bits to set 02, 03, 04 as outputs
str r5, [r4]        ; set pins 02, 03, 04 as OUTPUT
                    ; set pin 09 as INPUT

mov r5, #0x1c       ; bits [2..4] = 1
mov r6, #0          ; count = 0 (lsl 2, to align bits)
loop:               ; will keep repeating

str r5, [r4, #0x28] ; clear all pins
str r6, [r4, #0x1c] ; set pins

ldr r1, [r4, #0x34] ; r1 = input bits
and r1, r1, #0x200  ; only bit 9 (current state)
cmp r1, #0x200      ; check current state
beq end             ; if off, count
add r6, r6, #4      ; count++
end:

ldr r0, =0xfff000   ; set delay = 0xfff000
delay:              ; begin delay
sub r0, r0, #1
cmp r0, #0
bne delay           ; end delay

cmp r6, #32
blt loop              ; if >= 32 break

andeq r0,r0,r0

inf:
b inf
