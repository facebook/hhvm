/* JSON_parser.c */

/* 2005-12-30 */

/*
Copyright (c) 2005 JSON.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

The Software shall be used for Good, not Evil.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "hphp/runtime/ext/json/ext_json.h"

namespace HPHP {

void utf16_to_utf8(StringBuffer &buf, unsigned short utf16) {
  if (utf16 < 0x80) {
    buf.append((char)utf16);
  } else if (utf16 < 0x800) {
    buf.append((char)(0xc0 | (utf16 >> 6)));
    buf.append((char)(0x80 | (utf16 & 0x3f)));
  } else if ((utf16 & 0xfc00) == 0xdc00
             && buf.size() >= 3
             && ((unsigned char)buf.data()[buf.size() - 3]) == 0xed
             && ((unsigned char)buf.data()[buf.size() - 2] & 0xf0) == 0xa0
             && ((unsigned char)buf.data()[buf.size() - 1] & 0xc0) == 0x80) {
    /* found surrogate pair */
    unsigned long utf32;

    utf32 = (((buf.data()[buf.size() - 2] & 0xf) << 16)
             | ((buf.data()[buf.size() - 1] & 0x3f) << 10)
             | (utf16 & 0x3ff)) + 0x10000;
    buf.resize(buf.size() - 3);

    buf.append((char)(0xf0 | (utf32 >> 18)));
    buf.append((char)(0x80 | ((utf32 >> 12) & 0x3f)));
    buf.append((char)(0x80 | ((utf32 >> 6) & 0x3f)));
    buf.append((char)(0x80 | (utf32 & 0x3f)));
  } else {
    buf.append((char)(0xe0 | (utf16 >> 12)));
    buf.append((char)(0x80 | ((utf16 >> 6) & 0x3f)));
    buf.append((char)(0x80 | (utf16 & 0x3f)));
  }
}

}
