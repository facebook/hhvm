/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_BSTRING_H_
#define incl_HPHP_BSTRING_H_

#include <stdlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// chrcaseeq performs a case insensitive comparison and returns true if the
// characters are equal, false otherwise
inline bool chrcaseeq(char left, char right) {
  char k = left ^ right;
  if (k == 0) return true;
  if (k != 32) return false;
  k = left | right;
  return (k >= 'a' && k <= 'z');
}

// chrcasecmp performs a case insensitive comparison and returns < 0 if left
// is less than right, > 0 if left is greater than right, and 0 if the
// characters are equal
inline int chrcasecmp(char left, char right) {
  if (left == right) return 0;
  if (left >= 'A' && left <= 'Z') left += 32;
  if (right >= 'A' && right <= 'Z') right += 32;
  return (int)(unsigned char)left - (int)(unsigned char)right;
}

// Given two binary strings of equal length, bstrcaseeq does a case insensitive
// comparison and returns true if the strings are equal, false otherwise
bool bstrcaseeq(const char* left, const char* right, size_t n);

// Given two binary strings (possibly of different lengths), bstrcasecmp
// does a case insensitive comparison and returns < 0 if left is less than
// right, > 0 if left is greater than right, and 0 if the strings are equal
int bstrcasecmp(const char* left, size_t leftSize,
                const char* right, size_t rightSize);

// Given a binary string haystack and a character needle, bstrcasechr performs
// a case insensitive search and returns a pointer to the first occurrence of
// needle in haystack, or NULL if needle is not part of haystack
char* bstrcasechr(const char* haystack, char needle, size_t haystackSize);

// Given two binary strings haystack and needle, bstrcasestr performs a case
// insensitive search and returns a pointer to the first occurrence of needle
// in haystack, or NULL if needle is not part of haystack. If needleSize is 0,
// this function will return haystack.
char* bstrcasestr(const char* haystack, size_t haystackSize,
                  const char* needle, size_t needleSize);

// Given a binary strings haystack and a character needle, bstrcasestr performs
// a case insensitive search and returns a pointer to the last occurrence of
// needle in haystack, or NULL if needle is not part of haystack
char* bstrrcasechr(const char* haystack, char needle, size_t haystackSize);

// Given two binary strings haystack and needle, bstrcasestr performs a case
// insensitive search and returns a pointer to the last occurrence of needle in
// haystack, or NULL if needle is not part of haystack. If needleSize is 0,
// this function returns haystack + haystackSize.
char* bstrrcasestr(const char* haystack, size_t haystackSize,
                   const char* needle, size_t needleSize);

// Given two binary strings haystack and needle, bstrrstr performs a case
// sensitive search and returns a pointer to the last occurrence of needle in
// haystack, or NULL if needle is not part of haystack. If needleSize is 0,
// this function returns haystack + haystackSize.
char* bstrrstr(const char* haystack, size_t haystackSize,
               const char* needle, size_t needleSize);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_BSTRING_H_
