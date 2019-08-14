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

#ifndef incl_HPHP_STRING_FUNCTORS_H_
#define incl_HPHP_STRING_FUNCTORS_H_

#include "hphp/runtime/base/type-string.h"
#include "hphp/util/assertions.h"

namespace HPHP {

struct hphp_string_hash {
  size_t operator()(const String& s) const {
    return s.get()->hash();
  }
};

struct hphp_string_same {
  bool operator()(const String& s1, const String& s2) const {
    return s1.get()->same(s2.get());
  }
};

struct hphp_string_isame {
  bool operator()(const String& s1, const String& s2) const {
    return s1.get()->isame(s2.get());
  }
};

struct StringDataHashCompare {
  bool equal(const StringData *s1, const StringData *s2) const {
    assertx(s1 && s2);
    return s1->same(s2);
  }
  size_t hash(const StringData *s) const {
    assertx(s);
    return s->hash();
  }
};

struct StringDataHashICompare {
  bool equal(const StringData *s1, const StringData *s2) const {
    assertx(s1 && s2);
    return s1->isame(s2);
  }
  size_t hash(const StringData *s) const {
    assertx(s);
    return s->hash();
  }
};

struct StringDataPairHashICompare {
  bool equal(const std::pair<const StringData*, const StringData*>& p1,
             const std::pair<const StringData*, const StringData*>& p2) const {
    assertx(p1.first && p1.second && p2.first && p2.second);
    return p1.first->isame(p2.first) && p1.second->isame(p2.second);
  }
  size_t hash(const std::pair<const StringData*, const StringData*>& p) const {
    assertx(p.first && p.second);
    return folly::hash::hash_combine(p.first->hash(), p.second->hash());
  }
};

}
#endif
