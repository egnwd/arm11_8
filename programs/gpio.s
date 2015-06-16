ldr r4, =0x20200000 ; load base GPIO Address
ldr r5, =0x1240     ; bits to set 02, 03, 04 as outputs
str r5, [r4]        ; set pins 02, 03, 04 as OUTPUT
                    ; set pin 09 as INPUT

mov r7, #0x200      ; previous state is ON
mov r5, #0x1c       ; bits [2..4] = 1
mov r6, #0          ; count = 0 (lsl 2, to align bits)

counterLoop:        ; will keep repeating

str r5, [r4, #0x28] ; clear all pins
str r6, [r4, #0x1c] ; set pins

wait:
ldr r1, [r4, #0x34] ; r1 = input bits
and r1, r1, #0x200  ; only bit 9 (current state)
eor r1, r1, #0x200  ; !new state
tst r1, r7          ; previous && !new
eor r7, r1, #0x200  ; previous state = new state
beq wait            ; if previous && !new break

ldr r0, =0x138000   ; set delay = 0x138000
delay:              ; begin delay
sub r0, r0, #1
cmp r0, #0
bne delay           ; end delay

add r6, r6, #4      ; count++

cmp r6, #32
blt counterLoop     ; if >= 32 break

andeq r0,r0,r0

inf:
b inf
