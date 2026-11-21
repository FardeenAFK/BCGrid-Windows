// BC(c)Grid - UI strings.
// After Effects / Premiere read parameter names in the platform's legacy code page:
// MacRoman on macOS, Windows-1252 on Windows. Accented letters are escaped per platform
// so this file stays plain ASCII.
#pragma once

#if defined(_WIN32)
  #define BCG_i_ "\xED"   // i acute, Windows-1252
  #define BCG_o_ "\xF3"   // o acute, Windows-1252
  #define BCG_COPY "\xA9" // copyright sign, Windows-1252
#else
  #define BCG_i_ "\x92"   // i acute, MacRoman
  #define BCG_o_ "\x97"   // o acute, MacRoman
  #define BCG_COPY "\xA9" // copyright sign, MacRoman
#endif

// Registration strings: must stay identical to the macOS build so projects move between platforms.
#define BCG_NAME_UTF8      "BC\xC2\xA9Grid"
#define BCG_MATCH_NAME     "BillyCreative Grid Transition"
#define BCG_CATEGORY       "BillyCreative"
#define BCG_SUPPORT_URL    "https://billycreative.com"

#define BCG_NAME_DISPLAY   "BC" BCG_COPY "Grid"
#define BCG_DESCRIPTION    "Transici" BCG_o_ "n en cuadr" BCG_i_ "cula por BillyCreative."

#define STR_SECOND_CLIP    "Segundo clip"
#define STR_PROGRESS       "Progreso"
#define STR_COLUMNS        "Columnas"
#define STR_ROWS           "Filas"
#define STR_LINES          "L" BCG_i_ "neas"
#define STR_SHOW           "Mostrar"
#define STR_THICKNESS      "Grosor de l" BCG_i_ "neas (px)"
#define STR_LINE_COLOR     "Color de l" BCG_i_ "neas"
#define STR_PATTERN        "Patr" BCG_o_ "n"
#define STR_PATTERN_ITEMS  "Aleatorio|Filas|Columnas|Diagonal|Desde el centro|Hacia el centro"
#define STR_SEED           "Semilla aleatoria"
#define STR_FLASH          "Destello"
#define STR_ENABLED        "Activado"
#define STR_FLASH_COLOR    "Color del destello"
#define STR_FLASH_AMOUNT   "Intensidad del destello"
#define STR_FLASH_FRAMES   "Fotogramas del destello"
