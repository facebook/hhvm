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

/*
 * This file is responsible for declaring the various types in type
 * declaration macros that type-system-detail.h will use to generate
 * the treps automatically.
 */

// Uncounted non-union types which do not have Opt variants.
#define HHBBC_TYPE_DECL_SINGLE_UNCOUNTED_NO_OPT(X)        \
  X(Uninit)                                               \
  X(InitNull)                                             \

// Counted non-union types which do not have Opt variants (none right
// now).
#define HHBBC_TYPE_DECL_SINGLE_COUNTED_NO_OPT(X)

// Uncounted non-union types which do have Opt variants
#define HHBBC_TYPE_DECL_SINGLE_UNCOUNTED(X)     \
  X(False)                                      \
  X(True)                                       \
  X(Int)                                        \
  X(Dbl)                                        \
  X(Cls)                                        \
  X(LazyCls)                                    \
  X(Func)                                       \
  X(ClsMeth)                                    \
  X(EnumClassLabel)                             \

// Counted non-union types which do have Opt variants
#define HHBBC_TYPE_DECL_SINGLE_COUNTED(X)       \
  X(Obj)                                        \
  X(Res)                                        \
  X(RFunc)                                      \
  X(RClsMeth)                                   \

// Types which have both counted and uncounted base types (but not
// emptiness information), and a union of both. This is just Str, as
// arrays are dealt with separately below.
#define HHBBC_TYPE_DECL_COUNTED_OR_UNCOUNTED(X, Y)    \
  X(Str, Y)                                           \

// "Array" types. These has both counted/uncounted and empti/non-empty
// base type variations, and various unions between them.
#define HHBBC_TYPE_DECL_ARRAY(X, Y)             \
  X(Vec, Y)                                     \
  X(Dict, Y)                                    \
  X(Keyset, Y)                                  \

// Unions of other types which do not have Opt variants (because they
// already contain BInitNull).
#define HHBBC_TYPE_DECL_UNION_NO_OPT(X)         \
  X(Null, BUninit|BInitNull)                    \
  X(InitPrim, BInitNull|BBool|BNum)             \
  X(Prim, BUninit|BInitPrim)                    \

// Unions of other types which do have Opt variants
#define HHBBC_TYPE_DECL_UNION(X)                \
  X(Bool, BFalse|BTrue)                         \
  X(Num, BInt|BDbl)                             \
  X(ArrKey, BInt|BStr)                          \
  X(UncArrKey, BInt|BSStr)                      \

// Unions of array types. These require special care because arrays
// are already unions and we need to generate unions of all
// sub-unions.
#define HHBBC_TYPE_DECL_ARRAY_UNION(X, Y)       \
  X(KVish, Y, Vec, Dict)                        \
  X(ArrLike, Y, Vec, Dict, Keyset)              \

//////////////////////////////////////////////////////////////////////

// Actually generate the treps

#define HHBBC_TYPE_SYSTEM_DETAIL_H
#include "hphp/hhbbc/type-system-detail.h"
#undef HHBBC_TYPE_SYSTEM_DETAIL_H

//////////////////////////////////////////////////////////////////////

namespace HPHP::HHBBC {

// trep helper functions:

constexpr trep operator~(trep a) {
  return static_cast<trep>(~static_cast<int64_t>(a));
}

constexpr trep operator&(trep a, trep b) {
  return static_cast<trep>(static_cast<int64_t>(a) & b);
}

constexpr trep operator|(trep a, trep b) {
  return static_cast<trep>(static_cast<int64_t>(a) | b);
}

constexpr trep operator-(trep a, trep b) {
  return static_cast<trep>(static_cast<int64_t>(a) - b);
}

constexpr const trep& operator&=(trep&a, trep b) {
  a = a & b;
  return a;
}

constexpr const trep& operator|=(trep&a, trep b) {
  a = a | b;
  return a;
}

constexpr const trep& operator-=(trep& a, trep b) {
  a = a - b;
  return a;
}

constexpr bool couldBe(trep a, trep b) {
  return a & b;
}
constexpr bool subtypeOf(trep a, trep b) {
  return (a & b) == a;
}
constexpr bool subtypeAmong(trep a, trep b, trep c) {
  return (a & b) == (a & c);
}

//////////////////////////////////////////////////////////////////////

}
