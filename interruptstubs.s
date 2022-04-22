.set     IRQ_BASE, 0x20

.section .text

.extern _ZN17InterruptsManager15handleInterruptEhj
.global _ZN17InterruptsManager22ignoreInterruptRequestEv

.macro HandleInterruptException num
.global _ZN17InterruptsManager16handleInterruptEhjException\num\()Ev
_ZN17InterruptsManager16handleInterruptEhjException\num\()Ev:
    movb $\num, (interruptnumber)
    jmp int_bottom
.endm


.macro HandleInterruptRequest num
.global _ZN17InterruptsManager26handleInterruptRequest\num\()Ev
_ZN17InterruptsManager26handleInterruptRequest\num\()Ev:
    movb $\num + IRQ_BASE, (interruptnumber)
    jmp int_bottom
.endm

HandleInterruptRequest 0x00
HandleInterruptRequest 0x01

int_bottom:

    pusha
    pushl       %ds
    pushl       %es
    pushl       %fs
    pushl       %gs

    call        _ZN17InterruptsManager15handleInterruptEhj

    mov %eax, %esp

    popl       %gs
    popl       %fs
    popl       %es
    popl       (interruptnumber)
    popl       %esp
    popl       %ds
    popa



_ZN17InterruptsManager22ignoreInterruptRequestEv:
    iret

.data
    interruptnumber: .byte 0