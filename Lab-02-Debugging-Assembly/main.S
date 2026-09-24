@ Target Processor Type
.cpu cortex-m3
@ ISA Type
.thumb
@ Assembly syntax to use
.syntax unified

@ Make main visible outside of this file
.global main 

@ Place some 32-bit values in memory
.section .data
counter:           .word 0x00000000
counter_increment: .word 0x00000004

.section .text

@ implement a simple counter using variables in RAM
main:
    @use r0 as a pointer to memory address of counter_increment in RAM
    ldr r0, =counter_increment
    @load counter_increment value into register r1
    ldr r1, [r0]
    @use r0 as pointer to memory address of count variable in RAM
    ldr r0, =counter
count:
    @ load current counter value into r2, increment it, and store it
    ldr r2, [r0]
    add r2, r2, r1
    str r2, [r0]
    @ loop foreverd
    b count
    
