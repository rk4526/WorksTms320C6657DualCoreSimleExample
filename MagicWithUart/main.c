/* MagicWithUart - TMS320C6657 minimal UART echo + boot-magic (single file) */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <ti/csl/soc/c6657/src/cslr_device.h>
#include <ti/csl/cslr_uart.h>

/* ===== Select UART and clocks ===== */
#define UART_BASE    (CSL_UART_REGS)      /* UART0; use CSL_UART_B_REGS for UART1 */
/* Use 147.456 MHz for exact 115200 baud (divisor = 80) */
#define UART_CLK_HZ  147456000u
#define UART_BAUD    115200u

/* ===== Simple helpers ===== */
#define REG32(addr)          (*(volatile uint32_t *)(addr))
#define DEVICE_REG32_W(a,v)  (REG32(a) = (v))
#define DEVICE_REG32_R(a)    (REG32(a))

#define CHIP_LEVEL_REG   0x02620000u
#define KICK0            (CHIP_LEVEL_REG + 0x0038u)
#define KICK1            (CHIP_LEVEL_REG + 0x003Cu)

/* Boot-magic layout: each core has 0x?8FFFFC in its own L2; add core bits as below */
#define MAGIC_ADDR            0x008FFFFCu
#define BOOT_MAGIC_ADDR(x)   (MAGIC_ADDR + (1u<<28) + ((uint32_t)(x)<<24))
#define IPCGR(x)             (0x02620240u + (uint32_t)(x)*4u)

#define BOOT_MAGIC_NUMBER    0xBABEFACEu
#define MAX_CORE             2u

extern cregister volatile unsigned int DNUM;
extern far uint32_t _c_int00;

/* UART register overlay */
#define UART ((volatile CSL_UartRegs *)(uintptr_t)(UART_BASE))

/* ===== Prototypes ===== */
static void tiny_delay_us(uint32_t us);
static void uart_init(uint32_t uart_clk_hz, uint32_t baud);
static void uart_putc(char c);
static void uart_puts(const char *s);
static char uart_getc(void);

/* ===== Tiny busy-wait delay (TI compiler-safe) ===== */
static void tiny_delay_us(uint32_t us)
{
    volatile uint32_t n = us * 200u;  /* rough; just a short wait */
    while (n--) { /* no inline asm needed */ }
}

/* ===== UART tiny driver ===== */
static inline void uart_wait_tx_ready(void)
{
    while ((UART->LSR & (1u << 5)) == 0u) { }   /* THRE */
}

static void uart_putc(char c)
{
    uart_wait_tx_ready();
    UART->THR = (uint8_t)c;
}

static void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n') uart_putc('\r');
        uart_putc(*s++);
    }
}

static inline bool uart_rx_ready(void)
{
    return (UART->LSR & 0x01u) != 0u; /* DR */
}

static char uart_getc(void)
{
    while (!uart_rx_ready()) { }
    return (char)UART->RBR;
}

static void uart_init(uint32_t uart_clk_hz, uint32_t baud)
{
    UART->IER = 0x00u;          /* disable ints */
    UART->MDR = 0x07u;          /* disable mode */
    UART->LCR = 0x80u;          /* DLAB=1 */
    uint32_t divisor = (uart_clk_hz + (baud*8u)) / (16u * baud); /* rounded */
    UART->DLL = (uint8_t)(divisor & 0xFFu);
    UART->DLH = (uint8_t)((divisor >> 8) & 0xFFu);
    UART->LCR = 0x03u;          /* 8N1, DLAB=0 */
    UART->FCR = 0x07u;          /* FIFO enable + clear */
    UART->MCR = 0x00u;
    UART->MDR = 0x00u;          /* 16x UART mode */
    (void)UART->LSR; (void)UART->RBR; /* clear stale status */
}

/* ===== Core1 loop: write “alive” word then echo forever ===== */
uint32_t core0_data;
static void write_alive_and_echo(void)
{
    /* Optional: let Core0 see Core1 is alive (write Core0’s L2 global slot) */
    DEVICE_REG32_W(MAGIC_ADDR, BOOT_MAGIC_NUMBER);

    uart_puts("\n[Core1] Echo ready.\n");
    for (;;) {
        char c = uart_getc();
        uart_putc(c);
        if (c == '\r') uart_putc('\n');
        core0_data++;
    }
}

/* ===== Main ===== */
int main(void)
{
    uart_init(UART_CLK_HZ, UART_BAUD);
    uart_puts("\nC6657 UART echo (polled) @ 115200.\n");

    if (DNUM == 0u) {
        /* Unlock KICKs */
        DEVICE_REG32_W(KICK0, 0x83E70B13u);
        DEVICE_REG32_W(KICK1, 0x95A4F1E0u);

        /* Program Core1 entry and release it */
        DEVICE_REG32_W(BOOT_MAGIC_ADDR(1u), (uint32_t)(uintptr_t)&_c_int00);
        tiny_delay_us(1u);
        DEVICE_REG32_W(IPCGR(1u), 1u);
        tiny_delay_us(1000u);

        /* Core0 echo */
        uart_puts("[Core0] Echo ready.\n");
        for (;;) {
            char c = uart_getc();
            uart_putc(c);
            if (c == '\r') uart_putc('\n');
        }
    } else {
        /* Core1+ */
        write_alive_and_echo();
    }
}
