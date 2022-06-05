/* ** por compatibilidad se omiten tildes **
================================================================================
 TALLER System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Definicion de funciones de impresion por pantalla.
*/

#include "screen.h"

void print(const char* text, uint32_t x, uint32_t y, uint16_t attr) {
  ca(*p)[VIDEO_COLS] = (ca(*)[VIDEO_COLS])VIDEO; 
  int32_t i;
  for (i = 0; text[i] != 0; i++) {
    p[y][x].c = (uint8_t)text[i];
    p[y][x].a = (uint8_t)attr;
    x++;
    if (x == VIDEO_COLS) {
      x = 0;
      y++;
    }
  }
}

void print_dec(uint32_t numero, uint32_t size, uint32_t x, uint32_t y,
               uint16_t attr) {
  ca(*p)[VIDEO_COLS] = (ca(*)[VIDEO_COLS])VIDEO; 
  uint32_t i;
  uint8_t letras[16] = "0123456789";

  for (i = 0; i < size; i++) {
    uint32_t resto = numero % 10;
    numero = numero / 10;
    p[y][x + size - i - 1].c = letras[resto];
    p[y][x + size - i - 1].a = attr;
  }
}

void print_hex(uint32_t numero, int32_t size, uint32_t x, uint32_t y,
               uint16_t attr) {
  ca(*p)[VIDEO_COLS] = (ca(*)[VIDEO_COLS])VIDEO; 
  int32_t i;
  uint8_t hexa[8];
  uint8_t letras[16] = "0123456789ABCDEF";
  hexa[0] = letras[(numero & 0x0000000F) >> 0];
  hexa[1] = letras[(numero & 0x000000F0) >> 4];
  hexa[2] = letras[(numero & 0x00000F00) >> 8];
  hexa[3] = letras[(numero & 0x0000F000) >> 12];
  hexa[4] = letras[(numero & 0x000F0000) >> 16];
  hexa[5] = letras[(numero & 0x00F00000) >> 20];
  hexa[6] = letras[(numero & 0x0F000000) >> 24];
  hexa[7] = letras[(numero & 0xF0000000) >> 28];
  for (i = 0; i < size; i++) {
    p[y][x + size - i - 1].c = hexa[i];
    p[y][x + size - i - 1].a = attr;
  }
}

void screen_draw_box(uint32_t fInit, uint32_t cInit, uint32_t fSize,
                     uint32_t cSize, uint8_t character, uint8_t attr) {
  ca(*p)[VIDEO_COLS] = (ca(*)[VIDEO_COLS])VIDEO;
  uint32_t f;
  uint32_t c;
  for (f = fInit; f < fInit + fSize; f++) {
    for (c = cInit; c < cInit + cSize; c++) {
      p[f][c].c = character;
      p[f][c].a = attr;
    }
  }
}

#define rMid (ROWS / 2) 
#define cMid (COLS / 2) 
#define titilar 0x80

#define t_len 8
#define t_bg 0x00
#define t_fg 0x84
#define t_color (t_fg | t_bg | titilar)
#define t_lHeight 6
#define t_lWidth 5
#define t_fInit 5
#define t_cInit (4 * t_lWidth)

uint32_t strlen(char *s) {
  uint32_t res = 0;
  while (s[res] != '\0') res++;
  return res;
}

void print_title() { //U: Imprime en grande "GRUPO #1" y titilante
  char* title[t_len][t_lHeight] = {
    {
      " ### ",
      "#  # ",
      "#    ",
      "# ## ",
      "#  # ",
      " ##  "
    },
    {
      " ##  ",
      "#  # ",
      "#  # ",
      "###  ",
      "# #  ",
      "#  # ",
    },
    {
      "#  # ",
      "#  # ",
      "#  # ",
      "#  # ",
      "#  # ",
      " ##  ",
    },
    {
      " ##  ",
      "#  # ",
      "#  # ",
      "###  ",
      "#    ",
      "#    ",
    },
    {
      " ##  ",
      "#  # ",
      "#  # ",
      "#  # ",
      "#  # ",
      " ##  ",
    },
    {
      "     ",
      "     ",
      "     ",
      "     ",
      "     ",
      "     ",
    },
    {
      "     ",
      " # # ",
      "#####",
      " # # ",
      "#####",
      " # # ",
    },
    {
      "  # #",
      " ## #",
      "  # #",
      "  # #",
      "  #  ",
      "  # #",
    },
  };
  for (uint32_t i = 0; i < t_len; i++) {
    uint32_t x = i * t_lWidth + t_cInit;
    for (uint32_t j = 0; j < t_lHeight; j++) {
      uint32_t y = t_fInit + j;
      print(title[i][j], x, y, t_color);
    }
  }
}

#define n_len 3
#define n_fg 0x0F
#define n_bg 0x00
#define n_color (n_fg | n_bg)
#define n_fInit (t_fInit + t_fInit / 2 + t_lHeight)

void print_names() {
  char* names[n_len] = {
    "Galilo", 
    "Lu",
    "Juan",
  };
  for (uint32_t i = 0; i < n_len; i++) {
    uint32_t x = cMid - strlen(names[i]) / 2,
             y = n_fInit + i;
    print(names[i], x, y, n_color);
  }
}

void screen_draw_layout(void) {
  screen_draw_box(0, 0, 50, 80, '\0', 0x0); //A: Limpio el fondo

  print_title();
  print_names();
}
