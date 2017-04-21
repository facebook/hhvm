// Borrowed from Musl Libc, under the following license:
/*
 * Copyright (c) 2005-2014 Rich Felker, et al.
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
#ifndef incl_HPHP_UTIL_PORTABILITY_STRFMON_H_
#define incl_HPHP_UTIL_PORTABILITY_STRFMON_H_

#include "hphp/util/locale-portability.h"
#include <folly/Portability.h>

#include <stdint.h>

extern "C" ssize_t strfmon_l(char* __restrict s,
                             size_t n,
                             locale_t loc,
                             const char* __restrict fmt,
                             ...);
extern "C" ssize_t strfmon(char* __restrict s,
                           size_t n,
                           const char* __restrict fmt,
                           ...);

#endif
