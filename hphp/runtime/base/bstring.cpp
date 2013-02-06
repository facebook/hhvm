/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/bstring.h>
#include <util/util.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * The implementations below produce reasonably good machine code that
 * is comparable to the machine code for strcasecmp from glibc 4.4.0.
 */

bool bstrcaseeq(const char* left, const char* right, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    if (!chrcaseeq(left[i], right[i])) return false;
  }
  return true;
}

int bstrcasecmp(const char* left, size_t leftSize,
                const char* right, size_t rightSize) {
  size_t minSize = leftSize < rightSize ? leftSize : rightSize;
  for (size_t i = 0; i < minSize; ++i) {
    ssize_t ret = chrcasecmp(left[i], right[i]);
    if (ret) return ret; 
  }
  return (leftSize > rightSize) - (leftSize < rightSize);
}

char* bstrcasechr(const char* haystack, char needle, size_t haystackSize) {
  const char* haystackEnd = haystack + haystackSize;
  for (; haystack != haystackEnd; ++haystack) {
    if (chrcaseeq(*haystack, needle)) {
      return (char*)haystack;
    }
  }
  return nullptr;
}

HOT_FUNC
char* bstrcasestr(const char* haystack, size_t haystackSize,
                  const char* needle, size_t needleSize) {
  if (needleSize > haystackSize) {
    return nullptr;
  }
  const char* haystackLast = haystack + (haystackSize - needleSize);
  for (;;) {
    if (bstrcaseeq(haystack, needle, needleSize)) {
      return (char*)haystack;
    }
    if (haystack == haystackLast) return nullptr;
    ++haystack;
  }
}

char* bstrrcasechr(const char* haystack, char needle, size_t haystackSize) {
  if (haystackSize == 0) {
    return nullptr;
  }
  const char* haystackPtr = haystack + (haystackSize - 1);
  for (;;) {
    if (chrcaseeq(*haystackPtr, needle)) {
      return (char*)haystackPtr;
    }
    if (haystackPtr == haystack) return nullptr;
    --haystackPtr;
  }
}

char* bstrrcasestr(const char* haystack, size_t haystackSize,
                   const char* needle, size_t needleSize) {
  if (needleSize > haystackSize) {
    return nullptr;
  }
  const char* haystackPtr = haystack + (haystackSize - needleSize);
  for (;;) {
    if (bstrcaseeq(haystackPtr, needle, needleSize)) {
      return (char*)haystackPtr;
    }
    if (haystackPtr == haystack) return nullptr;
    --haystackPtr;
  }
}

char* bstrrstr(const char* haystack, size_t haystackSize,
               const char* needle, size_t needleSize) {
  if (needleSize > haystackSize) {
    return nullptr;
  }
  const char* haystackPtr = haystack + (haystackSize - needleSize);
  if (needleSize == 0) {
    return (char*)haystackPtr;
  }
  for (;;) {
    size_t j = 0;
    for (;;) {
      if (haystackPtr[j] != needle[j]) break;
      ++j;
      if (j == needleSize) return (char*)haystackPtr;
    }
    if (haystackPtr == haystack) return nullptr;
    --haystackPtr;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
