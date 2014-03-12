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

#ifndef incl_HPHP_RUNTIME_VM_TRANSLATOR_HOPT_VECTOR_TRANSLATOR_HELPERS_H_
#define incl_HPHP_RUNTIME_VM_TRANSLATOR_HOPT_VECTOR_TRANSLATOR_HELPERS_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/member-operations.h"

namespace HPHP {  namespace JIT { namespace {

#define CTX() cns(contextClass())

static const MInstrAttr Warn = MIA_warn;
static const MInstrAttr Unset = MIA_unset;
static const MInstrAttr Reffy = MIA_reffy;
static const MInstrAttr Define = MIA_define;
static const MInstrAttr None = MIA_none;
static const MInstrAttr WarnDefine = MInstrAttr(Warn | Define);
static const MInstrAttr DefineReffy = MInstrAttr(Define | Reffy);
static const MInstrAttr WarnDefineReffy = MInstrAttr(Warn | Define | Reffy);
#define WDU(attrs) (attrs & Warn) != 0, (attrs & Define) != 0, \
                   (attrs & Unset) != 0
#define WDRU(attrs) (attrs & Warn) != 0, (attrs & Define) != 0, \
                    (attrs & Reffy) != 0, (attrs & Unset) != 0

/* The following bunch of macros and functions are used to build up tables of
 * helper function pointers and determine which helper should be called based
 * on a variable number of bool and enum arguments. */

template<typename T> constexpr unsigned bitWidth() {
  return std::is_same<T, bool>::value ? 1
    : std::is_same<T, KeyType>::value ? 2
    : std::is_same<T, MInstrAttr>::value ? 4
    : sizeof(T) * CHAR_BIT;
}

// Determines the width in bits of all of its arguments
template<typename... T> unsigned multiBitWidth();
template<typename T, typename... Args>
inline unsigned multiBitWidth(T t, Args... args) {
  return bitWidth<T>() + multiBitWidth<Args...>(args...);
}
template<>
inline unsigned multiBitWidth() {
  return 0;
}

// Given the same arguments as multiBitWidth, buildBitmask will determine which
// index in the table corresponds to the provided parameters.
template<unsigned bit>
inline unsigned buildBitmask() {
  static_assert(bit < (sizeof(unsigned) * CHAR_BIT - 1), "Too many bits");
  return 0;
}
template<unsigned bit = 0, typename T, typename... Args>
inline unsigned buildBitmask(T c, Args... args) {
  unsigned bits = (unsigned)c & ((1u << bitWidth<T>()) - 1);
  return buildBitmask<bit + bitWidth<T>()>(args...) | bits << bit;
}

// FILL_ROW and BUILD_OPTAB* build up the static table of function pointers
#define FILL_ROW(nm, ...) do {                                  \
    OpFunc* dest = &optab[buildBitmask(__VA_ARGS__)];           \
    assert(*dest == nullptr);                                   \
    *dest = (OpFunc)MInstrHelpers::nm;                          \
  } while (false);

#define BUILD_OPTAB(...) BUILD_OPTAB_ARG(HELPER_TABLE(FILL_ROW), __VA_ARGS__)
#define BUILD_OPTAB_ARG(FILL_TABLE, ...)                                \
  static OpFunc* optab = nullptr;                                       \
  if (!optab) {                                                         \
    optab = (OpFunc*)calloc(1 << multiBitWidth(__VA_ARGS__), sizeof(OpFunc)); \
    FILL_TABLE                                                          \
  }                                                                     \
  unsigned idx = buildBitmask(__VA_ARGS__);                             \
  OpFunc opFunc = optab[idx];                                           \
  always_assert(opFunc);

// getKeyType determines the KeyType to be used as a template argument
// to helper functions.
inline KeyType getKeyType(const SSATmp* key) {
  auto DEBUG_ONLY keyType = key->type();
  assert(keyType.notBoxed());
  assert(keyType.isKnownDataType() || keyType.equals(Type::Cell));

  if (key->isA(Type::Str)) {
    return KeyType::Str;
  } else if (key->isA(Type::Int)) {
    return KeyType::Int;
  } else {
    return KeyType::Any;
  }
}

// like getKeyType, but for cases where we don't have an Int
// specialization for the helper.
inline KeyType getKeyTypeNoInt(const SSATmp* key) {
  auto kt = getKeyType(key);
  return kt == KeyType::Int ? KeyType::Any : kt;
}

// keyPtr is used by helper function implementations to convert a
// TypedValue passed by value into a TypedValue* suitable for passing
// to helpers from member_operations.h, which are prepared to handle
// int64 and StringData* keys in their key argument. This should be
// cleaned up to use the right types: #2174037
template<KeyType kt>
static inline TypedValue* keyPtr(TypedValue& key) {
  if (kt == KeyType::Any) {
    assert(tvIsPlausible(key));
    return &key;
  } else {
    return reinterpret_cast<TypedValue*>(key.m_data.num);
  }
}

} } }

#endif
