; fuji_atari7800_go.s -- the parts that must be exact 6502: counting
; MARIA's lines, and leaving for the cartridge's loader.
;
; Nothing here stores to $00-$1F: INPTCTRL stays as the BIOS left it.

        .export _fuji_a7800_detect_tv, _fuji_a7800_boot, _fuji_a7800_config

MWSYNC  = $24
MSTAT   = $28                   ; bit 7: vertical blank
CTRL    = $3C
POKEY   = $0450
CTLSWA  = $0281
CTLSWB  = $0283
REGSEL  = $0D00
REG_BOOTLOCK = $11
REG_TV  = $15
BOOTLOCK_MAGIC = $B5
LOADER_BOOT = $0600
LOADER_CONFIG = $0603

; NTSC MARIA draws 242 lines between blanks, PAL 292.
PAL_LINES = 270

        .code

; uint8_t fuji_a7800_detect_tv (void)
.proc _fuji_a7800_detect_tv
@in:    bit     MSTAT           ; into a blank...
        bpl     @in
@out:   bit     MSTAT           ; ...and out of it
        bmi     @out
        ldx     #0
        ldy     #0
@line:  sta     MWSYNC
        inx
        bne     @same
        iny
@same:  bit     MSTAT
        bpl     @line
        lda     #0              ; NTSC
        cpy     #>PAL_LINES
        bcc     @tell
        bne     @pal
        cpx     #<PAL_LINES
        bcc     @tell
@pal:   lda     #1
@tell:  sta     REGSEL+REG_TV
        ldx     #0
        rts
.endproc

; void fuji_a7800_boot (void) -- arm the staged image, then the loader.
.proc _fuji_a7800_boot
        lda     #BOOTLOCK_MAGIC
        sta     REGSEL+REG_BOOTLOCK
        jsr     quiet
        jmp     LOADER_BOOT
.endproc

; void fuji_a7800_config (void) -- back to CONFIG.
.proc _fuji_a7800_config
        jsr     quiet
        jmp     LOADER_CONFIG
.endproc

.proc quiet
        sei
        lda     #$7F            ; DMA off
        sta     CTRL
        lda     #0
        sta     POKEY+1         ; AUDC1-4: silent
        sta     POKEY+3
        sta     POKEY+5
        sta     POKEY+7
        sta     CTLSWA          ; joystick ports back to inputs
        sta     CTLSWB
        rts
.endproc
