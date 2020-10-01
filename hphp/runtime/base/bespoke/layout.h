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

// don't mix this file up with runtime/base/bespoke-layout.h !
#ifndef HPHP_BESPOKEDIR_LAYOUT_H_
#define HPHP_BESPOKEDIR_LAYOUT_H_

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/util/type-scan.h"

namespace HPHP {

namespace jit {

struct SSATmp;
struct Block;

namespace irgen { struct IRGS; }

} // namespace jit

namespace bespoke {

#define BESPOKE_LAYOUT_FUNCTIONS \
  X(size_t, heapSize, const ArrayData* ad) \
  X(size_t, align, const ArrayData* ad) \
  X(void, scan, const ArrayData* ad, type_scan::Scanner& scanner) \
  X(ArrayData*, escalateToVanilla, const ArrayData*, const char* reason) \
  X(void, convertToUncounted, ArrayData*, DataWalker::PointerMap* seen) \
  X(void, releaseUncounted, ArrayData*) \
  X(void, release, ArrayData*) \
  X(bool, isVectorData, const ArrayData*) \
  X(TypedValue, getInt, const ArrayData*, int64_t) \
  X(TypedValue, getStr, const ArrayData*, const StringData*) \
  X(TypedValue, getKey, const ArrayData*, ssize_t pos) \
  X(TypedValue, getVal, const ArrayData*, ssize_t pos) \
  X(ssize_t, getIntPos, const ArrayData*, int64_t) \
  X(ssize_t, getStrPos, const ArrayData*, const StringData*) \
  X(arr_lval, lvalInt, ArrayData* ad, int64_t k) \
  X(arr_lval, lvalStr, ArrayData* ad, StringData* k) \
  X(arr_lval, elemInt, ArrayData* ad, int64_t k) \
  X(arr_lval, elemStr, ArrayData* ad, StringData* k) \
  X(ArrayData*, setInt, ArrayData*, int64_t k, TypedValue v) \
  X(ArrayData*, setStr, ArrayData*, StringData* k, TypedValue v)\
  X(ArrayData*, removeInt, ArrayData*, int64_t) \
  X(ArrayData*, removeStr, ArrayData*, const StringData*) \
  X(ssize_t, iterBegin, const ArrayData*) \
  X(ssize_t, iterLast, const ArrayData*) \
  X(ssize_t, iterEnd, const ArrayData*) \
  X(ssize_t, iterAdvance, const ArrayData*, ssize_t) \
  X(ssize_t, iterRewind, const ArrayData*, ssize_t) \
  X(ArrayData*, append, ArrayData*, TypedValue v) \
  X(ArrayData*, prepend, ArrayData*, TypedValue v) \
  X(ArrayData*, pop, ArrayData*, Variant&) \
  X(ArrayData*, dequeue, ArrayData*, Variant&) \
  X(ArrayData*, copy, const ArrayData*) \
  X(ArrayData*, toDVArray, ArrayData*, bool copy) \
  X(ArrayData*, toHackArr, ArrayData*, bool copy)

struct LayoutFunctions {
#define X(Return, Name, Args...) Return (*Name)(Args);
  BESPOKE_LAYOUT_FUNCTIONS
#undef X
};

template <typename Array>
constexpr LayoutFunctions fromArray() {
  LayoutFunctions result;
#define X(Return, Name, Args...) result.Name = Array::Name;
  BESPOKE_LAYOUT_FUNCTIONS
#undef X
  return result;
}

/*
 * A bespoke::Layout can represent either both the concrete layout of a given
 * BespokeArray or some abstract type that's a union of concrete layouts.
 *
 * vtable() will be nullptr for an abstract layout. BespokeArray only admits
 * a concrete layout, but we may JIT code for abstract layouts.
 */
struct Layout {
  Layout(const std::string& description,
         const LayoutFunctions* vtable = nullptr);
  virtual ~Layout() {}

  /* bespoke indexes are 15 bits wide--when we store them in m_extra of
   * ArrayData we always set the sign bit--this allows us to test
   * (size>=constant * && vanilla()) in one go */
  static uint16_t constexpr kMaxIndex = (1 << 15) - 1;

  uint16_t index() const { return m_index; }
  const std::string& describe() const { return m_description; }
  const LayoutFunctions* vtable() const { return m_vtable; }

  //////////////////////////////////////////////////////////////////////
  // JIT support
  //////////////////////////////////////////////////////////////////////

  using SSATmp = jit::SSATmp;
  using Block = jit::Block;
  using IRGS = jit::irgen::IRGS;

  /*
   * Generate a new bespoke array by setting key to value in `base`, CoWing or
   * escalating as necessary.
   *
   * `base` is guaranteed to be bespoke, use the recipient's layout, and support
   * SetElem (i.e. is not a keyset)
   *
   * `key` is guaranteed to be valid for `base`'s array kind.
   */
  virtual SSATmp* emitSet(
    IRGS& env,
    SSATmp* base,
    SSATmp* key,
    SSATmp* val
  ) const;

  /*
   * Generate a new bespoke array by appending `val` (CoWing or escalating as
   * necessary.)
   *
   * `base` is guaranteed to be bespoke and use the recipient's layout
   *
   * `val` is guaranteed to be valid for `base`'s array kind.
   */
  virtual SSATmp* emitAppend(
    IRGS& env,
    SSATmp* base,
    SSATmp* val
  ) const;

  /*
   * Return the value at `key` in `base`, branching to `missing` if the key is
   * not present in the array.
   *
   * `base` is guaranteed to be bespoke and use the recipient's layout.
   *
   * `key` is guaranteed to be valid for `base`'s array kind.
   */
  virtual SSATmp* emitGet(
    IRGS& env,
    SSATmp* base,
    SSATmp* key,
    Block* taken
  ) const;

  /*
   * Return `true` iff the key `key` is present in `base`.
   *
   * `base` is guaranteed to be bespoke and use the recipient's layout
   *
   * `key` is guaranteed to an arraykey (i.e. Str or Int)
   */
  virtual SSATmp* emitIsset(
    IRGS& env,
    SSATmp* base,
    SSATmp* key
  ) const;

private:
  uint16_t m_index;
  std::string m_description;
  const LayoutFunctions* m_vtable;
};

const Layout* layoutForIndex(uint16_t index);

}}

#endif // HPHP_BESPOKEDIR_LAYOUT_H_
