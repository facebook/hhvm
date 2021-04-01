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

#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/member-operations.h"

#include "hphp/util/lock-free-ptr-wrapper.h"

#include <memory>

namespace HPHP { namespace jit { namespace {

/* The following bunch of macros and functions are used to build up tables of
 * helper function pointers and determine which helper should be called based
 * on a variable number of bool and enum arguments. */

template<typename T> constexpr unsigned bitWidth() {
  return std::is_same<T, bool>::value ? 1
    : std::is_same<T, KeyType>::value ? 2
    : std::is_same<T, MOpMode>::value ? 3
    : sizeof(T) * CHAR_BIT;
}

// Determines the width in bits of all of its arguments
template<typename... T> unsigned multiBitWidth();
template <typename T, typename... Args>
inline unsigned multiBitWidth(T /*t*/, Args... args) {
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
#define FILL_ROW(nm, ...)                               \
  new (&optab[buildBitmask(__VA_ARGS__)])               \
    CallSpec{CallSpec::direct(MInstrHelpers::nm)};

template<typename... Args>
std::tuple<typename std::remove_const<
             typename std::remove_reference<Args>::type>::type...>
make_type_tuple(Args...);

template<typename A, typename B>
struct assert_same {
  static_assert(std::is_same<A, B>::value, "Optab type mismatch");
};

#define MAKE_TYPE_TUPLE(...) decltype(make_type_tuple(__VA_ARGS__))

#define CHECK_TYPE_MATCH(nm, ...)                               \
  (void)assert_same<columns, MAKE_TYPE_TUPLE(__VA_ARGS__)>{};

#define BUILD_OPTAB(TABLE, ...)                                 \
  using columns = MAKE_TYPE_TUPLE(__VA_ARGS__);                 \
  TABLE(CHECK_TYPE_MATCH)                                       \
  using T =                                                     \
    std::aligned_storage_t<sizeof(CallSpec), alignof(CallSpec)>;\
  static auto const optabWrapper = [&] {                        \
    auto const n = 1 << multiBitWidth(__VA_ARGS__);             \
    auto optab = std::make_unique<T[]>(n);                      \
    if (debug) memset(optab.get(), 0, n * sizeof(T));       \
    TABLE(FILL_ROW);                                            \
    return optab;                                               \
  }();                                                          \
  auto const optab =                                            \
    reinterpret_cast<const CallSpec*>(optabWrapper.get());      \
  auto const& target = optab[buildBitmask(__VA_ARGS__)];        \
  assertx(target.address());

#define BUILD_OPTAB2(cond, table_t, table_f, ...) \
  auto& target = [&]() -> const CallSpec& {       \
    if ((cond)) {                                 \
      BUILD_OPTAB(table_t, __VA_ARGS__);          \
      return target;                              \
    }                                             \
    BUILD_OPTAB(table_f, __VA_ARGS__);            \
    return target;                                \
  }();

// getKeyType determines the KeyType to be used as a template argument
// to helper functions.
inline KeyType getKeyType(const SSATmp* key) {
  DEBUG_ONLY auto const keyType = key->type();
  assertx(keyType <= TCell);

  if (key->isA(TStr)) return KeyType::Str;
  if (key->isA(TInt)) return KeyType::Int;
  return KeyType::Any;
}

// like getKeyType, but for cases where we don't have an Int
// specialization for the helper.
inline KeyType getKeyTypeNoInt(const SSATmp* key) {
  auto kt = getKeyType(key);
  return kt == KeyType::Int ? KeyType::Any : kt;
}

}}}
