* fuji_bus_call.s -- 6809 assembly, varargs via PULU
*
* bool fuji_bus_call(uint8_t device, uint8_t fuji_cmd, uint8_t fields, ...)
*
* Varargs follow fields in this order:
*   0..4 uint8_t aux bytes   (count = fuji_field_numbytes(fields & $07))
*   const void *data, size_t data_length   if (fields & $08)
*   void *reply, size_t reply_length       if (fields & $10)
*   (data and reply are mutually exclusive)
*
* Return: bool in B (non-zero = success).
*
* After PSHS U / LEAU ,S / LEAU 4,U, U points at device (first arg).
* PULU walks U through each argument in order. Called functions preserve U,
* so position in the arg stream survives every JSR.
*
* Three values saved on S (all within 5-bit offset range):
*   [S+0] numbytes   (1 byte)
*   [S+1] fields     (1 byte, raw -- DATA/REPLY bits intact)
*   [S+2] header_len (1 byte)

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
        leau    ,s              * U = S
        leau    4,u             * U = first arg (device); skip saved U + return addr

*── Build header ──────────────────────────────────────────────────────────────

        pulu    b               * discard high byte
        pulu    b               * B = device

        cmpb    #FUJI_DEVICEID_NETWORK
        blo     fbc_not_network
        cmpb    #FUJI_DEVICEID_NETWORK_LAST
        bhi     fbc_not_network

        * Network: 3-byte header (opcode + unit + cmd)
        lda     #OP_NET
        sta     >_fb_header
        subb    #FUJI_DEVICEID_NETWORK-1    * unit = device - $70
        stb     >_fb_header+1
        pulu    b               * discard high byte of fuji_cmd
        pulu    b               * B = fuji_cmd
        stb     >_fb_header+2
        ldb     #3              * header_len
        bra     fbc_got_header

fbc_not_network:
        cmpb    #FUJI_DEVICEID_CLOCK
        beq     fbc_is_clock
        cmpb    #FUJI_DEVICEID_FUJINET
        beq     fbc_is_fuji
        * invalid device
        clrb
        puls    u
        rts

fbc_is_clock:
        lda     #OP_CLOCK
        bra     fbc_store_opcode
fbc_is_fuji:
        lda     #OP_FUJI
fbc_store_opcode:
        * Non-network: 2-byte header (opcode + cmd)
        sta     >_fb_header
        pulu    b               * discard high byte of fuji_cmd
        pulu    b               * B = fuji_cmd
        stb     >_fb_header+1
        ldb     #2              * header_len

fbc_got_header:
        pshs    b               * save header_len -- [S+0]=header_len

        pulu    b               * discard high byte of fields
        pulu    b               * B = fields (raw, DATA/REPLY bits intact)
        pshs    b               * save fields   -- [S+0]=fields [S+1]=header_len

        andb    #$07            * mask to numbytes index bits only
        ldx     #_fuji_field_numbytes_table
        ldb     b,x             * B = numbytes
        pshs    b               * save numbytes -- [S+0]=numbytes [S+1]=fields [S+2]=header_len

*── Collect aux bytes into buffer ─────────────────────────────────────────────

        beq     fbc_no_aux

        leax    _fuji_aux_buf,pcr

        pulu    b               * discard high byte
        pulu    b               * aux1
        stb     ,x+
        ldb     ,s              * reload numbytes
        cmpb    #2
        blo     fbc_no_aux
        pulu    b
        pulu    b               * aux2
        stb     ,x+
        ldb     ,s
        cmpb    #3
        blo     fbc_no_aux
        pulu    b
        pulu    b               * aux3
        stb     ,x+
        ldb     ,s
        cmpb    #4
        blo     fbc_no_aux
        pulu    b
        pulu    b               * aux4
        stb     ,x+

fbc_no_aux:

*── Send header and aux ───────────────────────────────────────────────────────

        pshs    x,b             * save X (aux_buf ptr) and B across bus_ready
        jsr     _bus_ready
        puls    x,b             * restore

        * dwwrite(&fb_header, header_len)
        ldb     2,s             * header_len
        clra
        pshs    d               * push header_len
        leax    _fb_header,pcr
        pshs    x               * push &fb_header
        jsr     _dwwrite
        leas    4,s

        * dwwrite(aux_buf, numbytes) if numbytes > 0
        ldb     ,s              * numbytes
        beq     fbc_no_aux_send
        clra
        pshs    d               * push numbytes
        leax    _fuji_aux_buf,pcr
        pshs    x               * push &aux_buf
        jsr     _dwwrite
        leas    4,s

fbc_no_aux_send:

*── Send data if flagged ──────────────────────────────────────────────────────

        ldb     1,s             * fields
        bitb    #FIELDS_DATA
        beq     fbc_no_data

        pulu    d               * data_ptr (16-bit, no promotion)
        tfr     d,x             * save in X
        pulu    d               * data_length
        pshs    d               * push data_length
        pshs    x               * push data_ptr
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
        pshs    x               * push reply_ptr
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
        pshs    d               * push reply_length
        pshs    x               * push reply_ptr
        jsr     _dwread
        leas    4,s
        bra     fbc_exit        * B = return value

*── Network path ──────────────────────────────────────────────────────────────

fbc_net_reply:
        clra
        ldb     >_fb_header+1   * unit
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
        pshs    x               * push reply_ptr
        clra
        ldb     >_fb_header+1   * unit
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
