/*
 * Copyright (c) 2009-2012 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "utf.h"
#include <string.h>

int utf8_encode(int32_t codepoint, char* buffer, int* size) {
  if (codepoint < 0)
    return -1;
  else if (codepoint < 0x80) {
    buffer[0] = (char)codepoint;
    *size = 1;
  } else if (codepoint < 0x800) {
    buffer[0] = 0xC0 + ((codepoint & 0x7C0) >> 6);
    buffer[1] = 0x80 + ((codepoint & 0x03F));
    *size = 2;
  } else if (codepoint < 0x10000) {
    buffer[0] = 0xE0 + ((codepoint & 0xF000) >> 12);
    buffer[1] = 0x80 + ((codepoint & 0x0FC0) >> 6);
    buffer[2] = 0x80 + ((codepoint & 0x003F));
    *size = 3;
  } else if (codepoint <= 0x10FFFF) {
    buffer[0] = 0xF0 + ((codepoint & 0x1C0000) >> 18);
    buffer[1] = 0x80 + ((codepoint & 0x03F000) >> 12);
    buffer[2] = 0x80 + ((codepoint & 0x000FC0) >> 6);
    buffer[3] = 0x80 + ((codepoint & 0x00003F));
    *size = 4;
  } else
    return -1;

  return 0;
}

int utf8_check_first(char byte) {
  unsigned char u = (unsigned char)byte;

  if (u < 0x80)
    return 1;

  if (0x80 <= u && u <= 0xBF) {
    /* second, third or fourth byte of a multi-byte
       sequence, i.e. a "continuation byte" */
    return 0;
  } else if (u == 0xC0 || u == 0xC1) {
    /* overlong encoding of an ASCII byte */
    return 0;
  } else if (0xC2 <= u && u <= 0xDF) {
    /* 2-byte sequence */
    return 2;
  }

  else if (0xE0 <= u && u <= 0xEF) {
    /* 3-byte sequence */
    return 3;
  } else if (0xF0 <= u && u <= 0xF4) {
    /* 4-byte sequence */
    return 4;
  } else { /* u >= 0xF5 */
    /* Restricted (start of 4-, 5- or 6-byte sequence) or invalid
       UTF-8 */
    return 0;
  }
}

int utf8_check_full(const char* buffer, int size, int32_t* codepoint) {
  int i;
  int32_t value = 0;
  unsigned char u = (unsigned char)buffer[0];

  if (size == 2) {
    value = u & 0x1F;
  } else if (size == 3) {
    value = u & 0xF;
  } else if (size == 4) {
    value = u & 0x7;
  } else
    return 0;

  for (i = 1; i < size; i++) {
    u = (unsigned char)buffer[i];

    if (u < 0x80 || u > 0xBF) {
      /* not a continuation byte */
      return 0;
    }

    value = (value << 6) + (u & 0x3F);
  }

  if (value > 0x10FFFF) {
    /* not in Unicode range */
    return 0;
  }

  else if (0xD800 <= value && value <= 0xDFFF) {
    /* invalid code point (UTF-16 surrogate halves) */
    return 0;
  }

  else if (
      (size == 2 && value < 0x80) || (size == 3 && value < 0x800) ||
      (size == 4 && value < 0x10000)) {
    /* overlong encoding */
    return 0;
  }

  if (codepoint)
    *codepoint = value;

  return 1;
}

const char* utf8_iterate(const char* buffer, int32_t* codepoint) {
  int count;
  int32_t value;

  if (!*buffer)
    return buffer;

  count = utf8_check_first(buffer[0]);
  if (count <= 0)
    return NULL;

  if (count == 1)
    value = (unsigned char)buffer[0];
  else {
    if (!utf8_check_full(buffer, count, &value))
      return NULL;
  }

  if (codepoint)
    *codepoint = value;

  return buffer + count;
}

int utf8_check_string(const char* string, int length) {
  int i;

  if (length == -1)
    length = strlen(string);

  for (i = 0; i < length; i++) {
    int count = utf8_check_first(string[i]);
    if (count == 0)
      return 0;
    else if (count > 1) {
      if (i + count > length)
        return 0;

      if (!utf8_check_full(&string[i], count, NULL))
        return 0;

      i += count - 1;
    }
  }

  return 1;
}

void utf8_fix_string(char* string, size_t length) {
  auto end = string + length;

  while (string < end) {
    int count = utf8_check_first(*string);
    if (count == 0) {
      // Invalid
      *string = '?';
      string++;
      continue;
    }
    if (count == 1) {
      // Valid ASCII character
      string++;
      continue;
    }

    if (count > end - string) {
      // UTF-8-encoded string claims to be longer than input. This means the
      // rest of the string is garbage.
      memset(string, '?', end - string);
      return;
    }

    if (!utf8_check_full(string, count, nullptr)) {
      // Invalid sequence
      while (count-- > 0) {
        *string = '?';
        string++;
      }
    } else {
      string += count;
    }
  }
}
