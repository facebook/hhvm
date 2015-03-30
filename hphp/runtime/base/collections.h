/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_COLLECTIONS_H_
#define incl_HPHP_COLLECTIONS_H_

#include "hphp/runtime/base/header-kind.h"

namespace HPHP {

struct CtResult { bool valid; CollectionType type; };
inline CtResult stringToCollectionType(const char* str, size_t len) {
  switch (len) {
    case 6:
      if (!strcasecmp(str, "hh\\set")) return {true, CollectionType::Set};
      if (!strcasecmp(str, "hh\\map")) return {true, CollectionType::Map};
      break;
    case 7:
      if (!strcasecmp(str, "hh\\pair")) return {true, CollectionType::Pair};
      break;
    case 9:
      if (!strcasecmp(str, "hh\\vector")) return {true, CollectionType::Vector};
      if (!strcasecmp(str, "hh\\immmap")) return {true, CollectionType::ImmMap};
      if (!strcasecmp(str, "hh\\immset")) return {true, CollectionType::ImmSet};
      break;
    case 12:
      if (!strcasecmp(str, "hh\\immvector")) {
        return {true, CollectionType::ImmVector};
      }
      break;
    default:
      break;
  }
  return {false};
}

inline CtResult stringToCollectionType(const std::string& s) {
  return stringToCollectionType(s.c_str(), s.size());
}
}

#endif
