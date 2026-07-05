* fuji_bus_call.s -- 6809 assembly, varargs via PULU
*
* bool fuji_bus_call(uint8_t device, uint8_t fuji_cmd, uint8_t fields, ...)
*
* Varargs follow fields in this order:
*   0..4 uint8_t aux bytes   (count = fuji_field_numbytes(fields))
*   const void *data, size_t data_length   if (fields & $08)
*   void *reply, size_t reply_length       if (fields & $10)
*   (data and reply are mutually exclusive)
*
* Return: bool in B (non-zero = success).
*
* Technique: after PSHS U / LEAU ,S / LEAU 4,U, U points at the first
* argument (device). PULU D consumes each successive argument and advances
* U by 2 (each arg is a promoted uint8_t or a 16-bit pointer). Called
* functions preserve U, so our position in the arg stream survives every JSR.
*
* Three values need to survive function calls and are saved on S:
*   [S+0] numbytes   (1 byte, PSHS B)
*   [S+1] fields     (1 byte, PSHS B)
*   [S+2] header_len (1 byte, PSHS B)
* All at 5-bit offsets from S -- 2-byte instructions throughout.

        section code

        extern  _bus_ready
        extern  _dwwrite
        extern  _dwread
        extern  _fuji_get_error
        extern  _fuji_get_response
        extern  _network_get_error
        extern  _network_get_response
        extern  _fuji_field_numbytes_table

        export  _fuji_bus_call
        export  _fb_header

FUJI_DEVICEID_NETWORK       equ     $71
FUJI_DEVICEID_NETWORK_LAST  equ     $78
FUJI_DEVICEID_CLOCK         equ     $45
FUJI_DEVICEID_FUJINET       equ     $70
OP_NET                      equ     $E3
OP_CLOCK                    equ     $E5
OP_FUJI                     equ     $E2
FIELDS_DATA                 equ     $08
FIELDS_REPLY                equ     $10

_fuji_bus_call:
        pshs    u               * save caller's U
        leau    ,s              * U = S (points to saved U on stack)
        leau    4,u             * U = first arg (device); skip saved U + return addr

* Load &fb_header into X early -- used for all three header field stores
* AND as the pointer argument to the first dwwrite call, saving a reload.
        ldx     #_fb_header

* ── Determine device type, build header ──────────────────────────────────────

        pulu    d               * B = device

        cmpb    #FUJI_DEVICEID_NETWORK
        blo     fbc_not_network
        cmpb    #FUJI_DEVICEID_NETWORK_LAST
        bhi     fbc_not_network

* Network device: 3-byte header (opcode + unit + cmd)
        lda     #OP_NET
        sta     ,x                      * fb_header.opcode = OP_NET
        subb    #FUJI_DEVICEID_NETWORK-1  * unit = device - $71 + 1 = device - $70
        stb     1,x                     * fb_header.fn.net.unit
        pulu    d               * B = fuji_cmd
        stb     2,x                     * fb_header.fn.net.cmd
        ldb     #3                      * header length for dwwrite
        bra     fbc_send_header

fbc_not_network:
        cmpb    #FUJI_DEVICEID_CLOCK
        beq     fbc_is_clock
        cmpb    #FUJI_DEVICEID_FUJINET
        beq     fbc_is_fuji

* Invalid device -- return false immediately (avoids long branch to far label)
        clrb
        puls    u
        rts

fbc_is_clock:
        lda     #OP_CLOCK
        bra     fbc_store_opcode

fbc_is_fuji:
        lda     #OP_FUJI

fbc_store_opcode:
* Non-network device: 2-byte header (opcode + cmd)
        sta     ,x                      * fb_header.opcode
        pulu    d               * B = fuji_cmd
        stb     1,x                     * fb_header.fn.cmd
        ldb     #2                      * header length for dwwrite

* ── Send header ──────────────────────────────────────────────────────────────

fbc_send_header:
	pshs	x,b
        jsr     _bus_ready              * no args, return value unused
	puls	x,b

* dwwrite(&fb_header, header_len)
* X = &fb_header (still valid from above), B = header_len
* CMOC convention: push rightmost arg first (header_len), then pointer
        clra                            * A:B = 0:header_len = uint16_t
        pshs    d                       * push header_len
        pshs    x                       * push &fb_header
        jsr     _dwwrite
        leas    4,s                     * caller cleanup

* ── Pack and send aux bytes ──────────────────────────────────────────────────
* Use B for numbytes throughout; use A for each aux value transfer.
* Fixed-offset stores to local aux array avoid pointer-spill overhead.

        pulu	d                       * fields
        ldx     #_fuji_field_numbytes_table
        ldb     b,x                     * B = fuji_field_numbytes_table[fields] (0..4)
        beq     fbc_no_aux              * if 0, nothing to send

	ldx	#_fuji_aux_buf
        pulu	d                       * aux1 (always present: numbytes >= 1)
        stb     ,x+                     * aux[0]
        cmpb    #2
        blo     fbc_send_aux
        lda     13,u                    * aux2
        sta     -3,u                    * aux[1]
        cmpb    #3
        blo     fbc_send_aux
        lda     15,u                    * aux3
        sta     -2,u                    * aux[2]
        cmpb    #4
        blo     fbc_send_aux
        lda     17,u                    * aux4 (3-byte: offset 17 > 15)
        sta     -1,u                    * aux[3]

fbc_send_aux:
* dwwrite(aux, numbytes) -- B = numbytes, A = last aux value (don't care)
        clra                            * A:B = 0:numbytes = uint16_t
        pshs    d
        leax    -4,u                    * &aux[0]
        pshs    x
        jsr     _dwwrite
        leas    4,s

fbc_no_aux:

*── Send header and aux bytes ─────────────────────────────────────────────────
* U preserved across all calls; our S-saved values survive via S preservation.

	pshs	x,b
        jsr     _bus_ready
	puls	x,b

        * dwwrite(&fb_header, header_len)
        ldb     2,s             * header_len
        clra
        pshs    d               * push header_len
        ldx     #_fb_header
        pshs    x               * push &fb_header
        jsr     _dwwrite
        leas    4,s

        * dwwrite(aux_buf, numbytes) if numbytes > 0
        ldb     ,s              * numbytes
        beq     fbc_no_aux_send
        clra
        pshs    d               * push numbytes
        leax    _fuji_aux_buf,pcr
        tfr     x,d
        pshs    d               * push &aux_buf
        jsr     _dwwrite
        leas    4,s

fbc_no_aux_send:

*── Send data if flagged ──────────────────────────────────────────────────────

        ldb     1,s             * fields
        bitb    #FIELDS_DATA
        beq     fbc_no_data

        pulu    d               * data_ptr
        tfr     d,x             * X = data_ptr
        pulu    d               * data_length
        pshs    d               * push data_length
        tfr     x,d
        pshs    d               * push data_ptr
        jsr     _dwwrite
        leas    4,s

fbc_no_data:

*── Dispatch for error check and reply ────────────────────────────────────────

        lda     >_fb_header     * opcode

        cmpa    #OP_CLOCK
        beq     fbc_clock_reply
        cmpa    #OP_NET
        beq     fbc_net_reply

*── Fuji path ─────────────────────────────────────────────────────────────────

        jsr     _fuji_get_error
        tstb
        bne     fbc_return_false

        ldb     1,s             * fields
        bitb    #FIELDS_REPLY
        beq     fbc_return_true

        pulu    d               * reply_ptr
        tfr     d,x
        pulu    d               * reply_length
        pshs    d               * push reply_length
        tfr     x,d
        pshs    d               * push reply_ptr
        jsr     _fuji_get_response
        leas    4,s
        bra     fbc_exit        * B = return value

*── Clock path ────────────────────────────────────────────────────────────────

fbc_clock_reply:
        ldb     1,s             * fields
        bitb    #FIELDS_REPLY
        beq     fbc_return_true

        pulu    d               * reply_ptr
        tfr     d,x
        pulu    d               * reply_length
        pshs    d
        tfr     x,d
        pshs    d
        jsr     _dwread
        leas    4,s
        bra     fbc_exit        * B = return value

*── Network path ──────────────────────────────────────────────────────────────

fbc_net_reply:
        ldb     >_fb_header+1   * unit
        clra
        pshs    d               * push unit
        jsr     _network_get_error
        leas    2,s
        tstb
        bne     fbc_return_false

        ldb     1,s             * fields
        bitb    #FIELDS_REPLY
        beq     fbc_return_true

        pulu    d               * reply_ptr
        tfr     d,x
        pulu    d               * reply_length
        pshs    d               * push reply_length
        tfr     x,d
        pshs    d               * push reply_ptr
        ldb     >_fb_header+1   * unit
        clra
        pshs    d               * push unit
        jsr     _network_get_response
        leas    6,s
        tstb
        bne     fbc_return_false
        * fall through to return true

*── Return paths ──────────────────────────────────────────────────────────────

fbc_return_true:
        ldb     #1
        bra     fbc_exit

fbc_return_false:
        clrb

fbc_exit:
        leas    3,s             * discard saved numbytes + fields + header_len
        puls    u               * restore caller's U
        rts                     * caller cleans up args

        endsection

        section bss
_fb_header:
        rmb     3               * opcode(1) + unit/cmd(1) + cmd(1)
_fuji_aux_buf:
        rmb     4               * aux byte staging buffer
        endsection

        end
