.cpu cortex-m0
.thumb
.syntax unified

.global _start

@ Vector Table: placed at address 0x0, tells the processor
@ where to find the code it should execute out of reset
.section .text
.word 0x20001000    @ stack pointer initialization value - place in RAM
.word _start         @ reset pointer - calls 'reset' function out of reset

.thumb_func
_start:
    b main          @ branch to 'main' function
    