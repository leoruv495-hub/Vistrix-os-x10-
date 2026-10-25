#include <stdint.h>
#include "mouse.h"

#define MULTIBOOT_BOOT_MAGIC 0x2BADB002
#define MB_INFO_FRAMEBUFFER (1u << 12)

static volatile uint32_t *fb;
static uint32_t pitch, screen_w, screen_h;
static int mx = 400, my = 300;
static uint32_t saved[16][12];
static int cursor_visible;

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void serial_init(void)
{
    outb(0x3F9, 0);
    outb(0x3FB, 0x80);
    outb(0x3F8, 3);
    outb(0x3F9, 0);
    outb(0x3FB, 3);
    outb(0x3FA, 0xC7);
    outb(0x3FC, 0x0B);
}

static void serial_write(const char *s)
{
    while (*s) {
        for (unsigned int n = 0; n < 1000000; n++)
            if (inb(0x3FD) & 0x20) break;
        outb(0x3F8, (uint8_t)*s++);
    }
}

static uint32_t rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

static void pixel(int x, int y, uint32_t color)
{
    if (!fb || x < 0 || y < 0 ||
        (uint32_t)x >= screen_w || (uint32_t)y >= screen_h)
        return;

    volatile uint32_t *row =
        (volatile uint32_t *)((volatile uint8_t *)fb + (uint32_t)y * pitch);
    row[x] = color;
}

static void rect(int x, int y, int w, int h, uint32_t color)
{
    for (int yy = y; yy < y + h; yy++)
        for (int xx = x; xx < x + w; xx++)
            pixel(xx, yy, color);
}

static void line(int x, int y, int w, uint32_t color)
{
    rect(x, y, w, 1, color);
}

/* Font 5x7 essenziale: lettere e numeri usati dal desktop. */
static const uint8_t *glyph(char c)
{
    static const uint8_t blank[5] = {0,0,0,0,0};
    static const uint8_t A[5] = {0x7E,0x11,0x11,0x11,0x7E};
    static const uint8_t D[5] = {0x7F,0x41,0x41,0x22,0x1C};
    static const uint8_t E[5] = {0x7F,0x49,0x49,0x49,0x41};
    static const uint8_t I[5] = {0,0x41,0x7F,0x41,0};
    static const uint8_t K[5] = {0x7F,0x08,0x14,0x22,0x41};
    static const uint8_t N[5] = {0x7F,0x02,0x04,0x08,0x7F};
    static const uint8_t O[5] = {0x3E,0x41,0x41,0x41,0x3E};
    static const uint8_t R[5] = {0x7F,0x09,0x19,0x29,0x46};
    static const uint8_t S[5] = {0x46,0x49,0x49,0x49,0x31};
    static const uint8_t T[5] = {0x01,0x01,0x7F,0x01,0x01};
    static const uint8_t V[5] = {0x1F,0x20,0x40,0x20,0x1F};
    static const uint8_t X[5] = {0x63,0x14,0x08,0x14,0x63};
    static const uint8_t zero[5] = {0x3E,0x45,0x49,0x51,0x3E};
    static const uint8_t one[5] = {0,0x21,0x7F,0x01,0};
    switch (c) {
        case 'A': return A; case 'D': return D; case 'E': return E;
        case 'I': return I; case 'K': return K; case 'N': return N;
        case 'O': return O; case 'R': return R; case 'S': return S;
        case 'T': return T; case 'V': return V; case 'X': return X;
        case '0': return zero; case '1': return one;
        default: return blank;
    }
}

static void text(int x, int y, const char *s, uint32_t color)
{
    while (*s) {
        const uint8_t *g = glyph(*s++);
        for (int col = 0; col < 5; col++)
            for (int row = 0; row < 7; row++)
                if (g[col] & (1u << row))
                    pixel(x + col, y + row, color);
        x += 7;
    }
}

static void desktop(void)
{
    rect(0, 0, (int)screen_w, (int)screen_h, rgb(0, 100, 150));

    /* Icone sul desktop. */
    rect(24, 25, 30, 27, rgb(220, 220, 220));
    rect(27, 28, 24, 21, rgb(30, 90, 180));
    text(20, 58, "VISTRIX", rgb(255,255,255));

    rect(24, 100, 30, 27, rgb(220, 220, 220));
    rect(27, 103, 24, 21, rgb(40, 150, 80));
    text(20, 133, "OS X10", rgb(255,255,255));

    /* Finestra principale stile classico. */
    int wx = (int)screen_w / 5;
    int wy = (int)screen_h / 6;
    int ww = (int)screen_w * 3 / 5;
    int wh = (int)screen_h * 3 / 5;

    rect(wx - 3, wy - 3, ww + 6, wh + 6, rgb(20, 20, 20));
    rect(wx, wy, ww, wh, rgb(192, 192, 192));
    rect(wx + 3, wy + 3, ww - 6, 24, rgb(0, 0, 128));
    text(wx + 10, wy + 11, "VISTRIX OS X10", rgb(255,255,255));

    rect(wx + 12, wy + 43, ww - 24, wh - 60, rgb(255,255,255));
    text(wx + 24, wy + 60, "VISTRIX", rgb(0,0,128));
    text(wx + 24, wy + 78, "OS X10", rgb(0,0,128));

    /* Barra delle applicazioni e pulsante Start. */
    int task_y = (int)screen_h - 34;
    rect(0, task_y, (int)screen_w, 34, rgb(192,192,192));
    line(0, task_y, (int)screen_w, rgb(255,255,255));
    rect(5, task_y + 5, 76, 24, rgb(192,192,192));
    rect(5, task_y + 5, 76, 2, rgb(255,255,255));
    rect(5, task_y + 5, 2, 24, rgb(255,255,255));
    rect(79, task_y + 5, 2, 24, rgb(80,80,80));
    text(15, task_y + 13, "START", rgb(0,0,0));

    rect((int)screen_w - 82, task_y + 5, 76, 24, rgb(192,192,192));
    text((int)screen_w - 70, task_y + 13, "OS X10", rgb(0,0,0));
}

static void cursor_restore(void)
{
    if (!cursor_visible) return;
    for (int y = 0; y < 16; y++)
        for (int x = 0; x < 12; x++)
            pixel(mx + x, my + y, saved[y][x]);
    cursor_visible = 0;
}

static void cursor_draw(void)
{
    if (!fb) return;
    for (int y = 0; y < 16; y++)
        for (int x = 0; x < 12; x++) {
            int px = mx + x, py = my + y;
            uint32_t *row = (uint32_t *)
                ((uint8_t *)fb + (uint32_t)py * pitch);
            saved[y][x] = (px >= 0 && py >= 0 &&
                (uint32_t)px < screen_w && (uint32_t)py < screen_h)
                ? row[px] : 0;
        }

    for (int y = 0; y < 16; y++) {
        int width = y / 2 + 1;
        for (int x = 0; x < width; x++) {
            pixel(mx + x, my + y, rgb(255,255,255));
            if (x == 0 || x == width - 1 || y == 15)
                pixel(mx + x, my + y, rgb(0,0,0));
        }
    }
    cursor_visible = 1;
}

void kernel_main(uint32_t magic, uint32_t mbi_addr)
{
    serial_init();
    serial_write("\r\nVistrix OS X10 Desktop Edition\r\n");

    if (magic != MULTIBOOT_BOOT_MAGIC || mbi_addr == 0) {
        serial_write("ERRORE: avvio Multiboot non valido.\r\n");
        for (;;) __asm__ volatile ("hlt");
    }

    uint32_t *mbi = (uint32_t *)mbi_addr;
    uint32_t flags = mbi[0];

    if ((flags & MB_INFO_FRAMEBUFFER) == 0 ||
        ((uint8_t *)mbi)[108] != 32 ||
        (uint32_t)mbi[22] == 0 || mbi[23] != 0) {
        serial_write("ERRORE: framebuffer 32-bit non disponibile.\r\n");
        serial_write("Kernel valido, ma desktop grafico non avviato.\r\n");
        for (;;) __asm__ volatile ("hlt");
    }

    fb = (volatile uint32_t *)(uintptr_t)mbi[22];
    pitch = mbi[24];
    screen_w = mbi[25];
    screen_h = mbi[26];

    if (!pitch || !screen_w || !screen_h ||
        screen_w > 1920 || screen_h > 1200 ||
        pitch < screen_w * 4) {
        serial_write("ERRORE: parametri framebuffer non validi.\r\n");
        for (;;) __asm__ volatile ("hlt");
    }

    desktop();
    mx = (int)screen_w / 2;
    my = (int)screen_h / 2;
    cursor_draw();

    serial_write("Framebuffer grafico inizializzato.\r\n");
    serial_write("Desktop Vistrix disegnato.\r\n");

    mouse_init();
    serial_write("Driver mouse PS/2 inizializzato.\r\n");
    serial_write("VISTRIX_DESKTOP_OK\r\n");

    for (;;) {
        int dx = 0, dy = 0;
        uint8_t buttons = 0;
        if (mouse_poll(&dx, &dy, &buttons)) {
            (void)buttons;
            cursor_restore();
            mx += dx;
            my += dy;

            if (mx < 0) mx = 0;
            if (my < 0) my = 0;
            if (mx > (int)screen_w - 12) mx = (int)screen_w - 12;
            if (my > (int)screen_h - 16) my = (int)screen_h - 16;

            cursor_draw();
        }
    }
}
