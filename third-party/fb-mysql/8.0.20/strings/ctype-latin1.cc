/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <stddef.h>
#include <sys/types.h>

#include "m_ctype.h"
#include "m_string.h"
#include "my_compiler.h"
#include "my_inttypes.h"

static const uchar ctype_latin1[] = {
    0,  32,  32,  32,  32,  32,  32,  32,  32,  32,  40,  40, 40, 40, 40, 32,
    32, 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32, 32, 32, 32, 32,
    32, 72,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16, 16, 16, 16, 16,
    16, 132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 16, 16, 16, 16, 16,
    16, 16,  129, 129, 129, 129, 129, 129, 1,   1,   1,   1,  1,  1,  1,  1,
    1,  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,  16, 16, 16, 16,
    16, 16,  130, 130, 130, 130, 130, 130, 2,   2,   2,   2,  2,  2,  2,  2,
    2,  2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,  16, 16, 16, 16,
    32, 16,  0,   16,  2,   16,  16,  16,  16,  16,  16,  1,  16, 1,  0,  1,
    0,  0,   16,  16,  16,  16,  16,  16,  16,  16,  16,  2,  16, 2,  0,  2,
    1,  72,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16, 16, 16, 16, 16,
    16, 16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16, 16, 16, 16, 16,
    16, 1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,  1,  1,  1,  1,
    1,  1,   1,   1,   1,   1,   1,   1,   16,  1,   1,   1,  1,  1,  1,  1,
    2,  2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,  2,  2,  2,  2,
    2,  2,   2,   2,   2,   2,   2,   2,   16,  2,   2,   2,  2,  2,  2,  2,
    2};

static const uchar to_lower_latin1[] = {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
    30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
    45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
    60,  61,  62,  63,  64,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106,
    107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
    122, 91,  92,  93,  94,  95,  96,  97,  98,  99,  100, 101, 102, 103, 104,
    105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
    165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 224, 225, 226,
    227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241,
    242, 243, 244, 245, 246, 215, 248, 249, 250, 251, 252, 253, 254, 223, 224,
    225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,
    255};

static const uchar to_upper_latin1[] = {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
    30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
    45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
    60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,
    75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
    90,  91,  92,  93,  94,  95,  96,  65,  66,  67,  68,  69,  70,  71,  72,
    73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,
    88,  89,  90,  123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
    165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
    195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
    210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 192,
    193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 247, 216, 217, 218, 219, 220, 221, 222,
    255};

static const uchar sort_order_latin1[] = {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
    30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
    45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
    60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,
    75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
    90,  91,  92,  93,  94,  95,  96,  65,  66,  67,  68,  69,  70,  71,  72,
    73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,
    88,  89,  90,  123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
    165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 65,  65,  65,
    65,  92,  91,  92,  67,  69,  69,  69,  69,  73,  73,  73,  73,  68,  78,
    79,  79,  79,  79,  93,  215, 216, 85,  85,  85,  89,  89,  222, 223, 65,
    65,  65,  65,  92,  91,  92,  67,  69,  69,  69,  69,  73,  73,  73,  73,
    68,  78,  79,  79,  79,  79,  93,  247, 216, 85,  85,  85,  89,  89,  222,
    255};

/*
  WL#1494 notes:

  We'll use cp1252 instead of iso-8859-1.
  cp1252 contains printable characters in the range 0x80-0x9F.
  In ISO 8859-1, these code points have no associated printable
  characters. Therefore, by converting from CP1252 to ISO 8859-1,
  one would lose the euro (for instance). Since most people are
  unaware of the difference, and since we don't really want a
  "Windows ANSI" to differ from a "Unix ANSI", we will:

   - continue to pretend the latin1 character set is ISO 8859-1
   - actually allow the storage of euro etc. so it's actually cp1252

  Also we'll map these five undefined cp1252 character:
    0x81, 0x8D, 0x8F, 0x90, 0x9D
  into corresponding control characters:
     U+0081, U+008D, U+008F, U+0090, U+009D.
  like ISO-8859-1 does. Otherwise, loading "mysqldump"
  output doesn't reproduce these undefined characters.
*/

static const unsigned short cs_to_uni[256] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008,
    0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F, 0x0010, 0x0011,
    0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001A,
    0x001B, 0x001C, 0x001D, 0x001E, 0x001F, 0x0020, 0x0021, 0x0022, 0x0023,
    0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C,
    0x002D, 0x002E, 0x002F, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035,
    0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E,
    0x003F, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, 0x0050,
    0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059,
    0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F, 0x0060, 0x0061, 0x0062,
    0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B,
    0x006C, 0x006D, 0x006E, 0x006F, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
    0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D,
    0x007E, 0x007F, 0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020,
    0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
    0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC,
    0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178, 0x00A0, 0x00A1,
    0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA,
    0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF, 0x00B0, 0x00B1, 0x00B2, 0x00B3,
    0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC,
    0x00BD, 0x00BE, 0x00BF, 0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5,
    0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE,
    0x00CF, 0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF, 0x00E0,
    0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9,
    0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF, 0x00F0, 0x00F1, 0x00F2,
    0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB,
    0x00FC, 0x00FD, 0x00FE, 0x00FF};
static const uchar pl00[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53,
    0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B,
    0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x00, 0x81, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8D, 0x00, 0x8F,
    0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x9D, 0x00, 0x00, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3,
    0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB,
    0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3,
    0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB,
    0xFC, 0xFD, 0xFE, 0xFF};
static const uchar pl01[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8C, 0x9C,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x8A, 0x9A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x9F, 0x00, 0x00, 0x00, 0x00, 0x8E, 0x9E, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00};
static const uchar pl02[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00};
static const uchar pl20[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x96, 0x97, 0x00, 0x00, 0x00,
    0x91, 0x92, 0x82, 0x00, 0x93, 0x94, 0x84, 0x00, 0x86, 0x87, 0x95, 0x00,
    0x00, 0x00, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x9B, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00};
static const uchar pl21[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x99, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00};
static const uchar *uni_to_cs[256] = {
    pl00,    pl01,    pl02,    nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    pl20,    pl21,    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

extern "C" {
static int my_mb_wc_latin1(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                           my_wc_t *wc, const uchar *str, const uchar *end) {
  if (str >= end) return MY_CS_TOOSMALL;

  *wc = cs_to_uni[*str];
  return (!wc[0] && str[0]) ? -1 : 1;
}

static int my_wc_mb_latin1(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                           my_wc_t wc, uchar *str, uchar *end) {
  const uchar *pl;

  if (str >= end) return MY_CS_TOOSMALL;

  if (wc > 0xFFFF) return MY_CS_ILUNI;

  pl = uni_to_cs[wc >> 8];
  str[0] = pl ? pl[wc & 0xFF] : '\0';
  return (!str[0] && wc) ? MY_CS_ILUNI : 1;
}
}  // extern "C"

static MY_CHARSET_HANDLER my_charset_handler = {nullptr, /* init */
                                                nullptr,
                                                my_mbcharlen_8bit,
                                                my_numchars_8bit,
                                                my_charpos_8bit,
                                                my_well_formed_len_8bit,
                                                my_lengthsp_8bit,
                                                my_numcells_8bit,
                                                my_mb_wc_latin1,
                                                my_wc_mb_latin1,
                                                my_mb_ctype_8bit,
                                                my_caseup_str_8bit,
                                                my_casedn_str_8bit,
                                                my_caseup_8bit,
                                                my_casedn_8bit,
                                                my_snprintf_8bit,
                                                my_long10_to_str_8bit,
                                                my_longlong10_to_str_8bit,
                                                my_fill_8bit,
                                                my_strntol_8bit,
                                                my_strntoul_8bit,
                                                my_strntoll_8bit,
                                                my_strntoull_8bit,
                                                my_strntod_8bit,
                                                my_strtoll10_8bit,
                                                my_strntoull10rnd_8bit,
                                                my_scan_8bit};

CHARSET_INFO my_charset_latin1 = {
    8,
    0,
    0,                              /* number    */
    MY_CS_COMPILED | MY_CS_PRIMARY, /* state */
    "latin1",                       /* cs name    */
    "latin1_swedish_ci",            /* name      */
    "cp1252 West European",         /* comment   */
    nullptr,                        /* tailoring */
    nullptr,                        /* coll_param */
    ctype_latin1,
    to_lower_latin1,
    to_upper_latin1,
    sort_order_latin1,
    nullptr,             /* uca          */
    cs_to_uni,           /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    1,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    1,                   /* mbminlen   */
    1,                   /* mbmaxlen   */
    1,                   /* mbmaxlenlen */
    0,                   /* min_sort_char */
    255,                 /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_handler,
    &my_collation_8bit_simple_ci_handler,
    PAD_SPACE};

/*
 * This file is the latin1 character set with German sorting
 *
 * The modern sort order is used, where:
 *
 * 'ä'  ->  "ae"
 * 'ö'  ->  "oe"
 * 'ü'  ->  "ue"
 * 'ß'  ->  "ss"
 */

/*
 * This is a simple latin1 mapping table, which maps all accented
 * characters to their non-accented equivalents.  Note: in this
 * table, 'ä' is mapped to 'A', 'ÿ' is mapped to 'Y', etc. - all
 * accented characters except the following are treated the same way.
 * Ü, ü, Ö, ö, Ä, ä
 */

static const uchar sort_order_latin1_de[] = {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
    30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
    45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
    60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,
    75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
    90,  91,  92,  93,  94,  95,  96,  65,  66,  67,  68,  69,  70,  71,  72,
    73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,
    88,  89,  90,  123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
    165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 65,  65,  65,
    65,  196, 65,  92,  67,  69,  69,  69,  69,  73,  73,  73,  73,  68,  78,
    79,  79,  79,  79,  214, 215, 216, 85,  85,  85,  220, 89,  222, 223, 65,
    65,  65,  65,  196, 65,  92,  67,  69,  69,  69,  69,  73,  73,  73,  73,
    68,  78,  79,  79,  79,  79,  214, 247, 216, 85,  85,  85,  220, 89,  222,
    89};

/*
  same as sort_order_latin_de, but maps ALL accented chars to unaccented ones
*/

static const uchar combo1map[] = {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
    30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
    45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
    60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,
    75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
    90,  91,  92,  93,  94,  95,  96,  65,  66,  67,  68,  69,  70,  71,  72,
    73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,
    88,  89,  90,  123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
    165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 65,  65,  65,
    65,  65,  65,  92,  67,  69,  69,  69,  69,  73,  73,  73,  73,  68,  78,
    79,  79,  79,  79,  79,  215, 216, 85,  85,  85,  85,  89,  222, 83,  65,
    65,  65,  65,  65,  65,  92,  67,  69,  69,  69,  69,  73,  73,  73,  73,
    68,  78,  79,  79,  79,  79,  79,  247, 216, 85,  85,  85,  85,  89,  222,
    89};

static const uchar combo2map[] = {
    0, 0, 0, 0, 0,  0, 0,  0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,
    0, 0, 0, 0, 0,  0, 0,  0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,
    0, 0, 0, 0, 0,  0, 0,  0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,
    0, 0, 0, 0, 0,  0, 0,  0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,
    0, 0, 0, 0, 0,  0, 0,  0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,
    0, 0, 0, 0, 0,  0, 0,  0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,
    0, 0, 0, 0, 0,  0, 0,  0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,
    0, 0, 0, 0, 0,  0, 0,  0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,
    0, 0, 0, 0, 69, 0, 0,  0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 69, 0,
    0, 0, 0, 0, 69, 0, 0,  83, 0, 0, 0, 0, 69, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,
    0, 0, 0, 0, 0,  0, 69, 0,  0, 0, 0, 0, 69, 0, 0, 0, 0};

/*
  Some notes about the following comparison rules:
  By definition, my_strnncoll_latin1_de() must work exactly as if one had called
  my_strnxfrm_latin1_de() on both strings and compared the resulting strings.

  This means that Ä must also match ÁE and Aè, because my_strxnfrm_latin1_de()
  will convert both to AE.

  The other option would be to not do any accent removal in
  sort_order_latin_de[] at all.
*/

extern "C" {
static int my_strnncoll_latin1_de(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                                  const uchar *a, size_t a_length,
                                  const uchar *b, size_t b_length,
                                  bool b_is_prefix) {
  const uchar *a_end = a + a_length;
  const uchar *b_end = b + b_length;
  uchar a_char, a_extend = 0, b_char, b_extend = 0;

  while ((a < a_end || a_extend) && (b < b_end || b_extend)) {
    if (a_extend) {
      a_char = a_extend;
      a_extend = 0;
    } else {
      a_extend = combo2map[*a];
      a_char = combo1map[*a++];
    }
    if (b_extend) {
      b_char = b_extend;
      b_extend = 0;
    } else {
      b_extend = combo2map[*b];
      b_char = combo1map[*b++];
    }
    if (a_char != b_char) return (int)a_char - (int)b_char;
  }
  /*
    A simple test of string lengths won't work -- we test to see
    which string ran out first
  */
  return ((a < a_end || a_extend) ? (b_is_prefix ? 0 : 1)
                                  : (b < b_end || b_extend) ? -1 : 0);
}

static int my_strnncollsp_latin1_de(
    const CHARSET_INFO *cs MY_ATTRIBUTE((unused)), const uchar *a,
    size_t a_length, const uchar *b, size_t b_length) {
  const uchar *a_end = a + a_length, *b_end = b + b_length;
  uchar a_char, a_extend = 0, b_char, b_extend = 0;
  int res;

  while ((a < a_end || a_extend) && (b < b_end || b_extend)) {
    if (a_extend) {
      a_char = a_extend;
      a_extend = 0;
    } else {
      a_extend = combo2map[*a];
      a_char = combo1map[*a++];
    }
    if (b_extend) {
      b_char = b_extend;
      b_extend = 0;
    } else {
      b_extend = combo2map[*b];
      b_char = combo1map[*b++];
    }
    if (a_char != b_char) return (int)a_char - (int)b_char;
  }
  /* Check if double character last */
  if (a_extend) return 1;
  if (b_extend) return -1;

  res = 0;
  if (a != a_end || b != b_end) {
    int swap = 1;
    /*
      Check the next not space character of the longer key. If it's < ' ',
      then it's smaller than the other key.
    */
    if (a == a_end) {
      /* put shorter key in a */
      a_end = b_end;
      a = b;
      swap = -1; /* swap sign of result */
      res = -res;
    }
    for (; a < a_end; a++) {
      if (*a != ' ') return (*a < ' ') ? -swap : swap;
    }
  }
  return res;
}

static size_t my_strnxfrm_latin1_de(const CHARSET_INFO *cs, uchar *dst,
                                    size_t dstlen, uint nweights,
                                    const uchar *src, size_t srclen,
                                    uint flags) {
  uchar *de = dst + dstlen;
  const uchar *se = src + srclen;
  uchar *d0 = dst;
  for (; src < se && dst < de && nweights; src++, nweights--) {
    uchar chr = combo1map[*src];
    *dst++ = chr;
    if ((chr = combo2map[*src]) && dst < de) {
      *dst++ = chr;
    }
  }
  return my_strxfrm_pad(cs, d0, dst, de, nweights, flags);
}

static void my_hash_sort_latin1_de(
    const CHARSET_INFO *cs MY_ATTRIBUTE((unused)), const uchar *key, size_t len,
    uint64 *nr1, uint64 *nr2) {
  const uchar *end;
  uint64 tmp1;
  uint64 tmp2;

  /*
    Remove end space. We have to do this to be able to compare
    'AE' and 'Ä' as identical
  */
  end = skip_trailing_space(key, len);

  tmp1 = *nr1;
  tmp2 = *nr2;

  for (; key < end; key++) {
    uint X = (uint)combo1map[(uint)*key];
    tmp1 ^= (uint64)((((uint)tmp1 & 63) + tmp2) * X) + (tmp1 << 8);
    tmp2 += 3;
    if ((X = combo2map[*key])) {
      tmp1 ^= (uint64)((((uint)tmp1 & 63) + tmp2) * X) + (tmp1 << 8);
      tmp2 += 3;
    }
  }

  *nr1 = tmp1;
  *nr2 = tmp2;
}
}  // extern "C"

static MY_COLLATION_HANDLER my_collation_german2_ci_handler = {
    nullptr, /* init */
    nullptr,
    my_strnncoll_latin1_de,
    my_strnncollsp_latin1_de,
    my_strnxfrm_latin1_de,
    my_strnxfrmlen_simple,
    my_like_range_simple,
    my_wildcmp_8bit,
    my_strcasecmp_8bit,
    my_instr_simple,
    my_hash_sort_latin1_de,
    my_propagate_complex};

CHARSET_INFO my_charset_latin1_german2_ci = {
    31,
    0,
    0,                               /* number    */
    MY_CS_COMPILED | MY_CS_STRNXFRM, /* state     */
    "latin1",                        /* cs name    */
    "latin1_german2_ci",             /* name      */
    "cp1252 West European",          /* comment   */
    nullptr,                         /* tailoring */
    nullptr,                         /* coll_param */
    ctype_latin1,
    to_lower_latin1,
    to_upper_latin1,
    sort_order_latin1_de,
    nullptr,             /* uca          */
    cs_to_uni,           /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    2,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    1,                   /* mbminlen   */
    1,                   /* mbmaxlen   */
    1,                   /* mbmaxlenlen */
    0,                   /* min_sort_char */
    247,                 /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_handler,
    &my_collation_german2_ci_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_latin1_bin = {
    47,
    0,
    0,                              /* number    */
    MY_CS_COMPILED | MY_CS_BINSORT, /* state     */
    "latin1",                       /* cs name    */
    "latin1_bin",                   /* name      */
    "cp1252 West European",         /* comment   */
    nullptr,                        /* tailoring */
    nullptr,                        /* coll_param */
    ctype_latin1,
    to_lower_latin1,
    to_upper_latin1,
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    cs_to_uni,           /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    1,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    1,                   /* mbminlen   */
    1,                   /* mbmaxlen   */
    1,                   /* mbmaxlenlen */
    0,                   /* min_sort_char */
    255,                 /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_handler,
    &my_collation_8bit_bin_handler,
    PAD_SPACE};
