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

#include <cstdint>

namespace HPHP {

// Note we don't use this macro to define the enum because this file is
// included by HackC and we don't want to expose HeaderKind to HackC.
#define COLLECTION_TYPES              \
  COL(Vector)                         \
  COL(Map)                            \
  COL(Set)                            \
  COL(Pair)                           \
  COL(ImmVector)                      \
  COL(ImmMap)                         \
  COL(ImmSet)

// These values must match the values of the corresponding variants in
// HeaderKind. This is statically enforced in header-kind.h
enum class CollectionType : uint8_t {
  Vector = 0x12,
  Map = 0x13,
  Set = 0x14,
  Pair = 0x15,
  ImmVector = 0x16,
  ImmMap = 0x17,
  ImmSet = 0x18,
};

inline constexpr bool isVectorCollection(CollectionType ctype) {
  return ctype == CollectionType::Vector || ctype == CollectionType::ImmVector;
}
inline constexpr bool isMapCollection(CollectionType ctype) {
  return ctype == CollectionType::Map || ctype == CollectionType::ImmMap;
}
inline constexpr bool isSetCollection(CollectionType ctype) {
  return ctype == CollectionType::Set || ctype == CollectionType::ImmSet;
}
inline constexpr bool isValidCollection(CollectionType ctype) {
  return uint8_t(ctype) >= uint8_t(CollectionType::Vector) &&
         uint8_t(ctype) <= uint8_t(CollectionType::ImmSet);
}
inline constexpr bool isMutableCollection(CollectionType ctype) {
  return ctype == CollectionType::Vector ||
         ctype == CollectionType::Map ||
         ctype == CollectionType::Set;
}
inline constexpr bool isImmutableCollection(CollectionType ctype) {
  return !isMutableCollection(ctype);
}

inline constexpr bool collectionAllowsIntStringKeys(CollectionType ctype) {
  return isSetCollection(ctype) || isMapCollection(ctype);
}

}
