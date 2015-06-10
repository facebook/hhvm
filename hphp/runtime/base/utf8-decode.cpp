/*
  +----------------------------------------------------------------------+
  | HipHop for PHP                                                       |
  +----------------------------------------------------------------------+
  | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
  | Copyright (c) 1997-2014 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Omar Kilani <omar@php.net>                                   |
  +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/utf8-decode.h"
#include "hphp/util/assertions.h"

namespace HPHP {

#define CHECK_LEN(pos, chars_need) ((m_strlen - (pos)) >= (chars_need))

/* valid as single byte character or leading byte */
static bool utf8_lead(unsigned char c) {
  return c < 0x80 || (c >= 0xC2 && c <= 0xF4);
}

/* whether it's actually valid depends on other stuff;
 * this macro cannot check for non-shortest forms, surrogates or
 * code points above 0x10FFFF */
static bool utf8_trail(unsigned char c) {
  return c >= 0x80 && c <= 0xBF;
}

#define MB_FAILURE(pos, advance) do { \
  m_cursor = pos + (advance); \
  return -1; \
} while (0)

// Inspired by ext/standard/html.c:get_next_char()
unsigned int UTF8To16Decoder::getNextChar() {
  int pos = m_cursor;
  unsigned int this_char = 0;

  assert(pos <= m_strlen);

  if (!CHECK_LEN(pos, 1))
    MB_FAILURE(pos, 1);

  /* We'll follow strategy 2. from section 3.6.1 of UTR #36:
   * "In a reported illegal byte sequence, do not include any
   *  non-initial byte that encodes a valid character or is a leading
   *  byte for a valid sequence." */
  unsigned char c = m_str[pos];
  if (c < 0x80) {
    this_char = c;
    pos++;
  } else if (c < 0xc2) {
    MB_FAILURE(pos, 1);
  } else if (c < 0xe0) {
    if (!CHECK_LEN(pos, 2))
      MB_FAILURE(pos, 1);

    if (!utf8_trail(m_str[pos + 1])) {
      MB_FAILURE(pos, utf8_lead(m_str[pos + 1]) ? 1 : 2);
    }
    this_char = ((c & 0x1f) << 6) | (m_str[pos + 1] & 0x3f);
    if (this_char < 0x80) { /* non-shortest form */
      MB_FAILURE(pos, 2);
    }
    pos += 2;
  } else if (c < 0xf0) {
    int avail = m_strlen - pos;

    if (avail < 3 ||
        !utf8_trail(m_str[pos + 1]) || !utf8_trail(m_str[pos + 2])) {
      if (avail < 2 || utf8_lead(m_str[pos + 1]))
        MB_FAILURE(pos, 1);
      else if (avail < 3 || utf8_lead(m_str[pos + 2]))
        MB_FAILURE(pos, 2);
      else
        MB_FAILURE(pos, 3);
    }

    this_char = ((c & 0x0f) << 12) | ((m_str[pos + 1] & 0x3f) << 6) |
                (m_str[pos + 2] & 0x3f);
    if (this_char < 0x800) { /* non-shortest form */
      MB_FAILURE(pos, 3);
    } else if (this_char >= 0xd800 && this_char <= 0xdfff) { /* surrogate */
      MB_FAILURE(pos, 3);
    }
    pos += 3;
  } else if (c < 0xf5) {
    int avail = m_strlen - pos;

    if (avail < 4 ||
        !utf8_trail(m_str[pos + 1]) || !utf8_trail(m_str[pos + 2]) ||
        !utf8_trail(m_str[pos + 3])) {
      if (avail < 2 || utf8_lead(m_str[pos + 1]))
        MB_FAILURE(pos, 1);
      else if (avail < 3 || utf8_lead(m_str[pos + 2]))
        MB_FAILURE(pos, 2);
      else if (avail < 4 || utf8_lead(m_str[pos + 3]))
        MB_FAILURE(pos, 3);
      else
        MB_FAILURE(pos, 4);
    }

    this_char = ((c & 0x07) << 18) | ((m_str[pos + 1] & 0x3f) << 12) |
                ((m_str[pos + 2] & 0x3f) << 6) | (m_str[pos + 3] & 0x3f);
    if (this_char < 0x10000 || this_char > 0x10FFFF) {
      /* non-shortest form or outside range */
      MB_FAILURE(pos, 4);
    }
    pos += 4;
  } else {
    MB_FAILURE(pos, 1);
  }

  m_cursor = pos;
  return this_char;
}

int UTF8To16Decoder::decodeTail() {
  int c = getNext();
  if (c < 0x10000) {
    return c;
  } else {
    c -= 0x10000;
    m_low_surrogate = (0xDC00 | (c & 0x3FF));
    return (0xD800 | (c >> 10));
  }
}

int UTF8To16Decoder::decodeAsUTF8() {
  if (m_index == m_cursor) {
    // validate the next char
    int c = getNext();
    if (c < 0) {
      return c;
    }
  }
  return m_str[m_index++] & 0xFF;
}

int UTF8To16Decoder::getNext() {
  int c = getNextChar();
  if (c < 0) {
  /*** BEGIN Facebook: json_utf8_loose ***/
    if (m_cursor > m_strlen) {
      return UTF8_END;
    }
    if (m_loose) {
      return '?';
    } else {
      return UTF8_ERROR;
    }
  /*** END Facebook: json_utf8_loose ***/
  } else {
    return c;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
