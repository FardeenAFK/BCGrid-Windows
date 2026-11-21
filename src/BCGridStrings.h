// BC(c)Grid - UI strings (English).
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
#define BCG_DESCRIPTION    "Grid transition by BillyCreative."

// English UI. Only display names changed from v1.0; parameter IDs are untouched,
// so projects made with the Spanish version still open with their settings.
#define STR_SECOND_CLIP    "Second clip"
#define STR_PROGRESS       "Progress"
#define STR_COLUMNS        "Columns"
#define STR_ROWS           "Rows"
#define STR_LINES          "Grid lines"
#define STR_SHOW           "Show"
#define STR_THICKNESS      "Line thickness (px)"
#define STR_LINE_COLOR     "Line color"
#define STR_PATTERN        "Pattern"
#define STR_PATTERN_ITEMS  "Random|Rows|Columns|Diagonal|From center|To center"
#define STR_SEED           "Random seed"
#define STR_FLASH          "Flash"
#define STR_ENABLED        "On"
#define STR_FLASH_COLOR    "Flash color"
#define STR_FLASH_AMOUNT   "Flash intensity"
#define STR_FLASH_FRAMES   "Flash length (frames)"
