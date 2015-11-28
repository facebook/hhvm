// Borrowed from Musl Libc, under the following license:
/*
 * Copyright ? 2005-2014 Rich Felker, et al.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef incl_HPHP_UTIL_PORTABILITY_GLOB_H_
#define incl_HPHP_UTIL_PORTABILITY_GLOB_H_

typedef struct {
  size_t gl_pathc;
  char **gl_pathv;
  size_t gl_offs;
  int __dummy1;
  void *__dummy2[5];
} glob_t;

extern "C" int glob(const char* __restrict,
                    int,
                    int(*)(const char*, int),
                    glob_t* __restrict);
extern "C" void globfree(glob_t*);

#define GLOB_ERR      0x01
#define GLOB_MARK     0x02
#define GLOB_NOSORT   0x04
#define GLOB_DOOFFS   0x08
#define GLOB_NOCHECK  0x10
#define GLOB_APPEND   0x20
#define GLOB_NOESCAPE 0x40
#define GLOB_PERIOD   0x80

#define GLOB_NOSPACE 1
#define GLOB_ABORTED 2
#define GLOB_NOMATCH 3
#define GLOB_NOSYS   4

#endif
