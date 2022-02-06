; Based on https://github.com/KimJorgensen/KungFuFlash/blob/master/launcher/crt0.crt.s

.segment "CRTSIG"
    .word cold_start
    .word cold_start
    .byte $C3,$C2,$CD,$38,$30

.export _exit
.export __STARTUP__ : absolute = 1
.export init_system

.import _main

.import initlib, donelib, copydata
.import zerobss
.import BSOUT
.import __RAM_START__, __RAM_SIZE__ ; Linker generated

.include "zeropage.inc"
.include "c64.inc"

CINT    = $ff81     ; Initialize screen editor
IOINIT  = $ff84     ; Initialize I/O devices
RESTOR  = $ff8a     ; Restore I/O vectors

; ------------------------------------------------------------------------
; Actual code

.code

init_system:
        lda #$00        ; Clear start of BASIC area
        sta $0800
        sta $0801
        sta $0802
        sta $8004       ; Trash autostart (if any)

init_system_no_clear:
        jsr IOINIT      ; Initialize I/O
        ; inc $d020
        ; jmp *-3
        jsr init_memory ; faster replacement for $ff87
        jsr RESTOR      ; Restore Kernal Vectors
        jmp CINT        ; Initialize screen editor

cold_start:
reset:
        ; same init stuff the kernel calls after reset
        ldx #$00
        stx $d016       ; 38 columns

        jsr init_system_no_clear

        ; Switch to second charset
        lda #$0e
        jsr BSOUT

        jsr zerobss
        jsr copydata

        ; and here
        ; Set argument stack ptr
        lda #<(__RAM_START__ + __RAM_SIZE__)
        sta sp
        lda #>(__RAM_START__ + __RAM_SIZE__)
        sta sp + 1

        jsr initlib
        cli
        jsr _main

_exit:
        jsr donelib

; ------------------------------------------------------------------------
; faster replacement for $ff87
init_memory:
        ; from KERNAL @ FD50:
        lda #$00
        tay
:
        sta $0002,y
        sta $0200,y
        sta $0300,y
        iny
        bne :-
        ldx #$3c
        ldy #$03
        stx $b2         ; pointer to datasette buffer
        sty $b3
        tay

        ; result from loop KERNAL @ FD6C:
        lda #$a0
        sta $c2
        sta $0284       ; pointer to end of BASIC area

        ; from KERNAL @ FD90:
        lda #$08
        sta $0282       ; pointer to beginning of BASIC area

        lda #$04
        sta $0288       ; high byte of screen memory address
        rts
