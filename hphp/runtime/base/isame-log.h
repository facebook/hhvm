/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once
#include <algorithm>
#include <cstring>
#include <folly/Range.h>
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/bstring.h"

namespace HPHP {

struct StringData;
bool tsame_log(const StringData*, const StringData*);
bool fsame_log(const StringData*, const StringData*);
int tstrcmp_log(const char* s1, const char* s2);
int fstrcmp_log(const char* s1, const char* s2);
int tstrcmp_log_slice(folly::StringPiece s1, folly::StringPiece s2);
int fstrcmp_log_slice(folly::StringPiece s1, folly::StringPiece s2);

inline int tstrcmp(const char* s1, const char* s2) {
  auto order = strcmp(s1, s2);
  if (order == 0 || RO::EvalLogTsameCollisions >= 2) return order;
  order = strcasecmp(s1, s2);
  if (order != 0 || RO::EvalLogTsameCollisions == 0) return order;
  return tstrcmp_log(s1, s2);
}

inline int fstrcmp(const char* s1, const char* s2) {
  auto order = strcmp(s1, s2);
  if (order == 0 || RO::EvalLogFsameCollisions >= 2) return order;
  order = strcasecmp(s1, s2);
  if (order != 0 || RO::EvalLogFsameCollisions == 0) return order;
  return fstrcmp_log(s1, s2);
}

inline int tstrcmp_slice(folly::StringPiece s1, folly::StringPiece s2) {
  auto minlen = std::min(s1.size(), s2.size());
  auto order = memcmp(s1.data(), s2.data(), minlen);
  if (order == 0 || RO::EvalLogTsameCollisions >= 2) {
    return s1.size() < s2.size() ? -1 :
           s1.size() > s2.size() ? 1 : 0;
  }
  order = bstrcasecmp(s1.data(), s1.size(), s2.data(), s2.size());
  if (order != 0 || RO::EvalLogTsameCollisions == 0) return order;
  return tstrcmp_log_slice(s1, s2);
}

inline int fstrcmp_slice(folly::StringPiece s1, folly::StringPiece s2) {
  auto minlen = std::min(s1.size(), s2.size());
  auto order = memcmp(s1.data(), s2.data(), minlen);
  if (order == 0 || RO::EvalLogFsameCollisions >= 2) {
    return s1.size() < s2.size() ? -1 :
           s1.size() > s2.size() ? 1 : 0;
  }
  order = bstrcasecmp(s1.data(), s1.size(), s2.data(), s2.size());
  if (order != 0 || RO::EvalLogFsameCollisions == 0) return order;
  return fstrcmp_log_slice(s1, s2);
}
}
