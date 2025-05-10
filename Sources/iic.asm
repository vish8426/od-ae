; Subroutines for controlling the IIC interface. HCS12 as slave is not supported
; by this code. Polled interface (not interrupt driven)
; Use #include to incorporate this code into your program
; Author - Tom Almy
; Date - January 28, 2005

; EXAMPLES OF USE:

; **** Always call IICINIT first to perform initialization.

; ***** Procedure to transmit:
;	ldaa	#control byte	; Address in bits 7-1, direction in bit 0 
				; (0-write/transmit)
;	jsr	IICSTART
;	brclr	IBCR #$20 fail	; branch if address NAKs. Bus is released

;	ldaa	#data byte	; Repeat this and next two lines for each byte
;	jsr	IICTRANSMIT
;	brclr	IBCR #$20 fail	; branch if data NAKs, bus is released

;	jsr	IICSTOP		; Release bus

; ***** Procedure to receive a single byte
;	ldaa	#control byte	; Address in bits 7-1, direction in bit 0 
				; (1-read/receive)
;	jsr	IICSTART
;	brclr	IBCR #$20 fail	; branch if address NAKs. Bus is released
;	jsr	IICRECEIVEONE	; bus is released afterwards
;	staa	data		; received byte in A

; ***** Procedure to receive two bytes
;	ldaa	#control byte	; Address in bits 7-1, direction in bit 0 
				; (1-read/receive)
;	jsr	IICSTART
;	brclr	IBCR #$20 fail	; branch if address NAKs. Bus is released
;	jsr	IICSWRCV	
;	jsr	IICRECEIVEM1
;	staa	data		; received byte in A
;	jsr	IICRECEIVELAST	; bus is released afterwards
;	staa	data		; received byte in A

; ***** Procedure to receive three or more bytes
;	ldaa	#control byte	; Address in bits 7-1, direction in bit 0 
				; (1-read/receive)
;	jsr	IICSTART
;	brclr	IBCR #$20 fail	; branch if address NAKs. Bus is released
;	jsr	IICSWRCV	

;	jsr	IICRECEIVE	; Repeat these two lines to receive all but
;	staa	data		; last two bytes

;	jsr	IICRECEIVEM1
;	staa	data		; received byte in A
;	jsr	IICRECEIVELAST	; bus is released afterwards
;	staa	data		; received byte in A

; A transmit can be immediately followed by a receive by omitting the call
; to IICSTOP in the transmit and calling IICRESTART instead of IICSTART for
; the receive.

IICINIT:	; Initialize IIC
	movb	#$23,IBFD	; Set to 100KHz operation
;	movb	#$35,IBFD	; Set to 20KHz operation
	movb	#$2,IBAD	; Slave address 1 (never address ourself, please!)
	bset	IBCR,#$81	; set IBEN
	rts

IICSTART:	; Issue start and address the slave. Control byte in A
		; Calling routine checks IBCR $20 clear for failure
	brset	IBSR,#$20,0+*	; wait for IBB flag to clear
	bset	IBCR,#$30	; Set XMIT and MASTER mode, emit START
	staa	IBDR		; Calling address
	brclr	IBSR,#$20,0+*	; Wait for IBB flag to set
IICRESPONSE:
	brclr	IBSR,#$2,0+*	; Wait for IBIF to set
	bclr	IBSR,#~$2	; Clear IBIF
	brset	IBSR,#$1,IICSTOP	; Stop if NAK
	rts

IICSTOP:	; Release bus and stop (only if transmitting)
	bclr	IBCR,#$20
	rts

IICRESTART:	; Issue restart and address the slave. Control byte in A
		; Calling routine checks IBCR $20 clear for failure
	bset	IBCR,#$04	; Generate restart
;	staa	IBDR
;	bra	IICRESPONSE

IICTRANSMIT:	; Data byte to send in A
		; Calling routine checks IBCR $20 clear for failure
	staa	IBDR
	bra	IICRESPONSE
	
IICSWRCV:       ; Switch to receive
	bclr    IBCR,#$10	; put in receive mode
	ldaa    IBDR            ; dummy read
	rts


IICRECEIVE:	; Call for all but last two bytes of a read sequence 3 or more bytes long
		; Data byte returned in A
	brclr	IBSR,#$2,0+*	; Wait for IBIF
	bclr	IBSR,#~2	; clear IBIF
	ldaa	IBDR		; read byte (and start next read)
	rts

IICRECEIVEM1:	; Call to receive next to last byte of multibyte receive sequence
		; Data byte returned in A
	brclr	IBSR,#$2,0+*	; Wait for IBIF
	bclr	IBSR,#~2	; clear IBIF
	bset	IBCR,#$8	; Disable ACK for last read
	ldaa	IBDR		; read byte (and start last read)
	rts

IICRECEIVELAST:	; Call to receive last byte of multibyte receive sequence
		; Data byte returned in A, Bus released
	brclr	IBSR,#$2,0+*	; Wait for IBIF
	bclr	IBSR,#~2	; clear IBIF
	bclr    IBCR,#$08        ; REENABLE ACK
	bclr	IBCR,#$20	; Generate STOP
	bset    IBCR,#$10       ; Set transmit
	ldaa	IBDR		; read byte
	rts

IICRECEIVEONE:	; Call to receive a single byte
		; Data byte returned in A, Bus released
	bset	IBCR,#$08	; Disable ACK
	bclr	IBCR,#$10	; Put in receive mode
	ldaa    IBDR            ; dummy read
	bra	IICRECEIVELAST
