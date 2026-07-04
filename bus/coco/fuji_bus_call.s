* fuji_bus_call.s
* Hand-written 6809 assembly implementation of fuji_bus_call.
*
* bool fuji_bus_call(uint8_t device, uint8_t fuji_cmd, uint8_t fields,
*                    uint8_t aux1, uint8_t aux2, uint8_t aux3, uint8_t aux4,
*                    const void *data, size_t data_length,
*                    void *reply, size_t reply_length)
*
* Return value: bool in B (non-zero = true/success) per CMOC convention.
*
* Stack frame after PSHS U / LEAU ,S / LEAS -4,S:
*
*   U-4         aux[0]          4-byte local array for aux payload
*   U-3         aux[1]
*   U-2         aux[2]
*   U-1         aux[3]
*   U+0  U+1    saved U
*   U+2  U+3    return address
*   U+4  U+5    device          uint8_t promoted, value in low byte (U+5)
*   U+6  U+7    fuji_cmd        value in low byte (U+7)
*   U+8  U+9    fields          value in low byte (U+9)
*   U+10 U+11   aux1            value in low byte (U+11)
*   U+12 U+13   aux2            value in low byte (U+13)
*   U+14 U+15   aux3            value in low byte (U+15)
*   U+16 U+17   aux4            value in low byte (U+17) *outside 5-bit offset*
*   U+18 U+19   data            pointer
*   U+20 U+21   data_length
*   U+22 U+23   reply           pointer
*   U+24 U+25   reply_length
*
* Offsets 0..15 from U use 5-bit indexed addressing (2-byte instruction).
* Offsets 16+ require 8-bit indexed addressing (3-byte instruction).
* Offsets marked with * above incur this extra byte.
*
* Calling convention (CMOC default):
*   - Arguments pushed right-to-left by caller; caller pops after return.
*   - Called functions may clobber A, B, X, CC but preserve U, Y, S, DP.
*   - Condition codes after JSR are NOT reliable; always TSTB before branching.
*   - JSR to external symbol = 3 bytes (vs LBSR = 4 bytes, saves 1 byte/call).

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

* ── Constants ────────────────────────────────────────────────────────────────
* Must match fujinet-bus.h / fujinet-commands.h

FUJI_DEVICEID_NETWORK       equ     $71
FUJI_DEVICEID_NETWORK_LAST  equ     $78
FUJI_DEVICEID_CLOCK         equ     $45
FUJI_DEVICEID_FUJINET       equ     $70

OP_NET                      equ     $E3
OP_CLOCK                    equ     $E5
OP_FUJI                     equ     $E2

* ── Function ─────────────────────────────────────────────────────────────────

_fuji_bus_call:
        pshs    u                       * save frame pointer
        leau    ,s                      * U = frame pointer
        leas    -4,s                    * reserve aux[4] local array

* Load &fb_header into X early -- used for all three header field stores
* AND as the pointer argument to the first dwwrite call, saving a reload.
        ldx     #_fb_header

        ldb     5,u                     * B = device (low byte of promoted uint16_t)

* ── Determine device type, build header ──────────────────────────────────────

        cmpb    #FUJI_DEVICEID_NETWORK
        blo     fbc_not_network
        cmpb    #FUJI_DEVICEID_NETWORK_LAST
        bhi     fbc_not_network

* Network device: 3-byte header (opcode + unit + cmd)
        lda     #OP_NET
        sta     ,x                      * fb_header.opcode = OP_NET
        subb    #FUJI_DEVICEID_NETWORK-1  * unit = device - $71 + 1 = device - $70
        stb     1,x                     * fb_header.fn.net.unit
        lda     7,u                     * fuji_cmd
        sta     2,x                     * fb_header.fn.net.cmd
        ldb     #3                      * header length for dwwrite
        bra     fbc_send_header

fbc_not_network:
        cmpb    #FUJI_DEVICEID_CLOCK
        beq     fbc_is_clock
        cmpb    #FUJI_DEVICEID_FUJINET
        beq     fbc_is_fuji

* Invalid device -- return false immediately (avoids long branch to far label)
        clrb
        leas    ,u
        puls    u,pc

fbc_is_clock:
        lda     #OP_CLOCK
        bra     fbc_store_opcode

fbc_is_fuji:
        lda     #OP_FUJI

fbc_store_opcode:
* Non-network device: 2-byte header (opcode + cmd)
        sta     ,x                      * fb_header.opcode
        lda     7,u                     * fuji_cmd
        sta     1,x                     * fb_header.fn.cmd
        ldb     #2                      * header length for dwwrite

* ── Send header ──────────────────────────────────────────────────────────────

fbc_send_header:
        jsr     _bus_ready              * no args, return value unused

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

        ldb     9,u                     * fields
        ldx     #_fuji_field_numbytes_table
        ldb     b,x                     * B = fuji_field_numbytes_table[fields] (0..4)
        beq     fbc_no_aux              * if 0, nothing to send

        lda     11,u                    * aux1 (always present: numbytes >= 1)
        sta     -4,u                    * aux[0]
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

* ── Send data payload if present ─────────────────────────────────────────────
* Load data pointer into X to avoid a second load when passing to dwwrite.

        ldx     18,u                    * data pointer (3-byte: offset 18 > 15)
        beq     fbc_no_data             * if NULL, skip
        ldd     20,u                    * data_length (3-byte: offset 20 > 15)
        beq     fbc_no_data             * if 0, skip
        pshs    d                       * push data_length
        pshs    x                       * push data pointer (already in X)
        jsr     _dwwrite
        leas    4,s

fbc_no_data:

* ── Dispatch on response/error handling ──────────────────────────────────────
* Read opcode from fb_header to determine which path to take.
* This avoids re-doing the device range check that we already did above.

        lda     >_fb_header             * opcode (extended addressing)

        cmpa    #OP_CLOCK
        beq     fbc_clock_reply

        cmpa    #OP_NET
        beq     fbc_net_reply

* ── Fuji device path ─────────────────────────────────────────────────────────

        jsr     _fuji_get_error         * returns error code in B
        tstb
        bne     fbc_return_false        * if error, return false

        ldx     22,u                    * reply pointer (3-byte: offset 22 > 15)
        beq     fbc_return_true         * if NULL, no reply expected -- return true

* fuji_get_response(reply, reply_length)
* push: reply_length (rightmost), then reply pointer
        ldd     24,u                    * reply_length (3-byte: offset 24 > 15)
        pshs    d
        pshs    x                       * reply pointer (still in X)
        jsr     _fuji_get_response
        leas    4,s
        bra     fbc_exit                * B = return value from fuji_get_response

* ── Clock path ───────────────────────────────────────────────────────────────

fbc_clock_reply:
        ldx     22,u                    * reply pointer
        beq     fbc_return_true         * if NULL, return true

* dwread(reply, reply_length)
        ldd     24,u                    * reply_length
        pshs    d
        pshs    x
        jsr     _dwread
        leas    4,s
        bra     fbc_exit                * B = return value from dwread

* ── Network path ─────────────────────────────────────────────────────────────

fbc_net_reply:
* network_get_error(unit)
* Push unit as promoted uint16_t: CLRA gives high byte 0, LDB gives unit in B
        ldb     >_fb_header+1           * unit (stored earlier in header setup)
        clra
        pshs    d                       * push unit as uint16_t
        jsr     _network_get_error
        leas    2,s
        tstb
        bne     fbc_return_false        * if error, return false

        ldx     22,u                    * reply pointer
        beq     fbc_return_true         * if NULL, return true

* network_get_response(unit, reply, reply_length)
* push: reply_length (rightmost), reply, unit (leftmost)
        ldd     24,u                    * reply_length
        pshs    d
        pshs    x                       * reply pointer (still in X)
        ldb     >_fb_header+1           * unit (reload; X may have been clobbered)
        clra
        pshs    d                       * push unit as uint16_t
        jsr     _network_get_response
        leas    6,s
        tstb
        bne     fbc_return_false        * if error, return false
                                        * fall through to return true

* ── Return paths ─────────────────────────────────────────────────────────────

fbc_return_true:
        ldb     #1
        bra     fbc_exit

fbc_return_false:
        clrb                            * fall through to exit

fbc_exit:
        leas    ,u
        puls    u,pc

        endsection

* ── Global data ──────────────────────────────────────────────────────────────

        section bss
_fb_header:
        rmb     3       * fujibus_header: opcode(1) + fn.net.unit(1) + fn.net.cmd/fn.cmd(1)
        endsection

        end
