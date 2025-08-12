/******************************************************************************
 * TMS320C6657 Linker Command File (CCS v9.3) - minimal, robust
 * - DDR3 for code/data by default
 * - L2SRAM used for stack, .vecs, and fast small data
 * - Works for a single-core build; duplicate this file per core as needed
 *****************************************************************************/

--entry_point=_c_int00
-c
-heap  0x2000
-stack 0x2000

MEMORY
{
    DDR3        (RWX) : origin = 0x80000000, length = 0x20000000  /* 512MB DDR3 */
    L2SRAM_C0   (RWX) : origin = 0x10800000, length = 0x00080000  /* 512KB Core0 L2 */
    L2SRAM_C1   (RWX) : origin = 0x11800000, length = 0x00080000  /* 512KB Core1 L2 */
}

/* === Pick which L2 to use for THIS build ================================ */
/* For Core0 build: */
#define L2SRAM     L2SRAM_C0
/* For Core1 build: comment the line above and uncomment: */
/* #define L2SRAM     L2SRAM_C1 */

/* === Section placement =================================================== */
SECTIONS
{
    /* Code + rodata in DDR3 (big) */
    .text        > DDR3
    .const       > DDR3
    .switch      > DDR3
    .rodata      > DDR3

    /* C/C++ initialization tables */
    .cinit       > DDR3
    .pinit       > DDR3
    .binit       > DDR3
    .init_array  > DDR3
    .fardata     > DDR3
    .far         > DDR3

    /* Vector table & CSL vectors in L2 so interrupts (if used later) are fast */
    .vecs        > L2SRAM  ALIGN(0x400)
    .csl_vect    > L2SRAM

    /* Fast small data, stack, and I/O buffers in L2 */
    .neardata    > L2SRAM
    .bss         > L2SRAM
    .cio         > L2SRAM         /* printf/puts buffer */
    .sysmem      > L2SRAM         /* malloc heap backing */
    .stack       > L2SRAM
    .args        > L2SRAM

    /* Optional: place any UART DMA/RX ring buffers explicitly */
    /* .uart_buf   > L2SRAM  ALIGN(128) */

    /* Keep copy tables if the tools emit them */
    .ti.decompress  > DDR3
    .c6xabi.exidx   > DDR3
    .c6xabi.extab   > DDR3
}
