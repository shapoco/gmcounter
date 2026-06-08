#pragma once

// Generated from ShapoFont
//   Pixel Count:
//     Effective:   804 px
//     Shrinked :   644 px
//   Estimated Foot Print:
//     Bitmap Data    :    85 Bytes (  7.08 Bytes/glyph)
//     Glyph Table    :    96 Bytes (  8.00 Bytes/glyph)
//     GFXfont Struct :    10 Bytes
//     Total          :   191 Bytes ( 15.92 Bytes/glyph)
//   Memory Efficiency:  4.209 px/Byte

#include <stdint.h>

#ifdef SHAPOFONT_INCLUDE_AVR_PGMSPACE
#include <avr/pgmspace.h>
#endif

#ifdef SHAPOFONT_INCLUDE_GFXFONT
#include <gfxfont.h>
#endif

#ifndef SHAPOFONT_PROGMEM
#ifdef PROGMEM
#define SHAPOFONT_PROGMEM PROGMEM
#else
#define SHAPOFONT_PROGMEM
#endif
#define SHAPOFONT_PROGMEM_SELF_DEFINED
#endif

#ifndef SHAPOFONT_GFXFONT_NAMESPACE
#define SHAPOFONT_GFXFONT_NAMESPACE
#define SHAPOFONT_GFXFONT_NAMESPACE_SELF_DEFINED
#endif

const uint8_t GmcDigit12Bitmaps[] SHAPOFONT_PROGMEM = {
  0x18, 0xC6, 0x63, 0x19, 0x8C, 0x66, 0x31, 0x80, 0x7B, 0xFC, 0xF3, 0xCF, 0x3C, 0xF3, 0xFD, 0xE0,
  0x37, 0xFB, 0x33, 0x33, 0x33, 0x7B, 0xFC, 0xC3, 0x1C, 0xE7, 0x38, 0xFF, 0xF0, 0xFF, 0xF1, 0x8C,
  0x79, 0xF0, 0xD3, 0xFD, 0xE0, 0x61, 0x86, 0x30, 0xDB, 0x6F, 0xFF, 0x18, 0x60, 0xFF, 0xFC, 0x30,
  0xFB, 0xF0, 0xD3, 0xFD, 0xE0, 0x30, 0xC6, 0x1E, 0xFF, 0x3C, 0xF3, 0xFD, 0xE0, 0xFF, 0xFC, 0xF3,
  0x18, 0x61, 0x8C, 0x30, 0xC0, 0x7B, 0xFC, 0xF3, 0x7B, 0xFC, 0xF3, 0xFD, 0xE0, 0x7B, 0xFC, 0xF3,
  0xCF, 0xF7, 0x86, 0x30, 0xC0,
};

const SHAPOFONT_GFXFONT_NAMESPACE GFXglyph GmcDigit12Glyphs[] SHAPOFONT_PROGMEM = {
  { 0x001C,  2,  2,  3,  0,  -2 },
  { 0x0000,  5, 12,  6,  0, -12 },
  { 0x0008,  6, 10,  7,  0, -11 },
  { 0x0010,  4, 10,  7,  0, -11 },
  { 0x0015,  6, 10,  7,  0, -11 },
  { 0x001D,  6, 10,  7,  0, -11 },
  { 0x0025,  6, 10,  7,  0, -11 },
  { 0x002D,  6, 10,  7,  0, -11 },
  { 0x0035,  6, 10,  7,  0, -11 },
  { 0x003D,  6, 10,  7,  0, -11 },
  { 0x0045,  6, 10,  7,  0, -11 },
  { 0x004D,  6, 10,  7,  0, -11 },
};

const SHAPOFONT_GFXFONT_NAMESPACE GFXfont GmcDigit12 SHAPOFONT_PROGMEM = {
  (uint8_t*)GmcDigit12Bitmaps,
  (GFXglyph*)GmcDigit12Glyphs,
  0x2E,
  0x39,
  15
};

#ifdef SHAPOFONT_PROGMEM_SELF_DEFINED
#undef SHAPOFONT_PROGMEM
#endif

#ifdef SHAPOFONT_GFXFONT_NAMESPACE_SELF_DEFINED
#undef SHAPOFONT_GFXFONT_NAMESPACE
#endif
