	include	"portio.inc"

;; extern int __FASTCALL__ port_getc_timeout(uint16_t timeout);
;; timeout is in HL
;; loop until data is available or timeout elapses
_port_getc_timeout:
wait_rx:
	in	a,(SIO_CTRL)	; get control register bits
	bit	SIO_RX_READY,a	; check if char is avail
	jr	nz,getc_done	; zero flag clear if data

	dec	hl		; decrement timeout
	ld	a,h
	or	l
	jr	nz,wait_rx	; try again

	ld	hl,0xFFFF	; timed out, return -1
	ret
getc_done:
	in	a,(SIO_DATA)	; read character
	ld	l,a
	ld	h,0

	ret
