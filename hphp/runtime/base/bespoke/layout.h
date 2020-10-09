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

#define BESPOKE_LAYOUT_FUNCTIONS(T) \
  X(size_t, HeapSize, const T* ad) \
  X(size_t, Align, const T* ad) \
  X(void, Scan, const T* ad, type_scan::Scanner& scanner) \
  X(ArrayData*, EscalateToVanilla, const T*, const char* reason) \
  X(void, ConvertToUncounted, T*, DataWalker::PointerMap* seen) \
  X(void, ReleaseUncounted, T*) \
  X(void, Release, T*) \
  X(bool, IsVectorData, const T*) \
  X(TypedValue, GetInt, const T*, int64_t) \
  X(TypedValue, GetStr, const T*, const StringData*) \
  X(TypedValue, GetKey, const T*, ssize_t pos) \
  X(TypedValue, GetVal, const T*, ssize_t pos) \
  X(ssize_t, GetIntPos, const T*, int64_t) \
  X(ssize_t, GetStrPos, const T*, const StringData*) \
  X(ssize_t, IterBegin, const T*) \
  X(ssize_t, IterLast, const T*) \
  X(ssize_t, IterEnd, const T*) \
  X(ssize_t, IterAdvance, const T*, ssize_t) \
  X(ssize_t, IterRewind, const T*, ssize_t) \
  X(arr_lval, LvalInt, T* ad, int64_t k) \
  X(arr_lval, LvalStr, T* ad, StringData* k) \
  X(arr_lval, ElemInt, T* ad, int64_t k) \
  X(arr_lval, ElemStr, T* ad, StringData* k) \
  X(ArrayData*, SetInt, T*, int64_t k, TypedValue v) \
  X(ArrayData*, SetStr, T*, StringData* k, TypedValue v)\
  X(ArrayData*, RemoveInt, T*, int64_t) \
  X(ArrayData*, RemoveStr, T*, const StringData*) \
  X(ArrayData*, Append, T*, TypedValue v) \
  X(ArrayData*, Pop, T*, Variant&) \
  X(ArrayData*, ToDVArray, T*, bool copy) \
  X(ArrayData*, ToHackArr, T*, bool copy) \
  X(ArrayData*, SetLegacyArray, T*, bool copy, bool legacy)

struct LayoutFunctions {
#define X(Return, Name, Args...) Return (*fn##Name)(Args);
  BESPOKE_LAYOUT_FUNCTIONS(ArrayData)
#undef X
};

/**
 * Provides an interface between LayoutFunctions, which exposes methods
 * accepting ArrayData*, and the bespoke array implementations, which expose
 * methods accepting their array types. In a debug build, it uses the bespoke
 * array's static As() function to convert from ArrayData* to the specific
 * bespoke array type. This As() function should perform any invariant
 * checking. In a non-debug build, a reinterpret_cast is used to avoid any
 * overhead from this wrapper.
 */
template <typename Array>
struct LayoutFunctionDispatcher {
  ALWAYS_INLINE static Array* Cast(ArrayData* ad) {
    return Array::As(ad);
  }

  ALWAYS_INLINE static const Array* Cast(const ArrayData* ad) {
    return Array::As(ad);
  }

  static size_t HeapSize(const ArrayData* ad) {
    return Array::HeapSize(Cast(ad));
  }
  static size_t Align(const ArrayData* ad) {
    return Array::Align(Cast(ad));
  }
  static void Scan(const ArrayData* ad, type_scan::Scanner& scanner) {
    return Array::Scan(Cast(ad), scanner);
  }
  static ArrayData* EscalateToVanilla(const ArrayData* ad, const char* reason) {
    return Array::EscalateToVanilla(Cast(ad), reason);
  }
  static void ConvertToUncounted(ArrayData* ad, DataWalker::PointerMap* seen) {
    return Array::ConvertToUncounted(Cast(ad), seen);
  }
  static void ReleaseUncounted(ArrayData* ad) {
    return Array::ReleaseUncounted(Cast(ad));
  }
  static void Release(ArrayData* ad) {
    return Array::Release(Cast(ad));
  }
  static bool IsVectorData(const ArrayData* ad) {
    return Array::IsVectorData(Cast(ad));
  }
  static TypedValue GetInt(const ArrayData* ad, int64_t k) {
    return Array::GetInt(Cast(ad), k);
  }
  static TypedValue GetStr(const ArrayData* ad, const StringData* k) {
    return Array::GetStr(Cast(ad), k);
  }
  static TypedValue GetKey(const ArrayData* ad, ssize_t pos) {
    return Array::GetKey(Cast(ad), pos);
  }
  static TypedValue GetVal(const ArrayData* ad, ssize_t pos) {
    return Array::GetVal(Cast(ad), pos);
  }
  static ssize_t GetIntPos(const ArrayData* ad, int64_t k) {
    return Array::GetIntPos(Cast(ad), k);
  }
  static ssize_t GetStrPos(const ArrayData* ad, const StringData* k) {
    return Array::GetStrPos(Cast(ad), k);
  }
  static arr_lval LvalInt(ArrayData* ad, int64_t k) {
    return Array::LvalInt(Cast(ad), k);
  }
  static arr_lval LvalStr(ArrayData* ad, StringData* k) {
    return Array::LvalStr(Cast(ad), k);
  }
  static arr_lval ElemInt(ArrayData* ad, int64_t k) {
    return Array::ElemInt(Cast(ad), k);
  }
  static arr_lval ElemStr(ArrayData* ad, StringData* k) {
    return Array::ElemStr(Cast(ad), k);
  }
  static ArrayData* SetInt(ArrayData* ad, int64_t k, TypedValue v) {
    return Array::SetInt(Cast(ad), k, v);
  }
  static ArrayData* SetStr(ArrayData* ad, StringData* k, TypedValue v){
    return Array::SetStr(Cast(ad), k, v);
  }
  static ArrayData* RemoveInt(ArrayData* ad, int64_t k) {
    return Array::RemoveInt(Cast(ad), k);
  }
  static ArrayData* RemoveStr(ArrayData* ad, const StringData* k) {
    return Array::RemoveStr(Cast(ad), k);
  }
  static ssize_t IterBegin(const ArrayData* ad) {
    return Array::IterBegin(Cast(ad));
  }
  static ssize_t IterLast(const ArrayData* ad) {
    return Array::IterLast(Cast(ad));
  }
  static ssize_t IterEnd(const ArrayData* ad) {
    return Array::IterEnd(Cast(ad));
  }
  static ssize_t IterAdvance(const ArrayData* ad, ssize_t pos) {
    return Array::IterAdvance(Cast(ad), pos);
  }
  static ssize_t IterRewind(const ArrayData* ad, ssize_t pos) {
    return Array::IterRewind(Cast(ad), pos);
  }
  static ArrayData* Append(ArrayData* ad, TypedValue v) {
    return Array::Append(Cast(ad), v);
  }
  static ArrayData* Pop(ArrayData* ad, Variant& v) {
    return Array::Pop(Cast(ad), v);
  }
  static ArrayData* ToDVArray(ArrayData* ad, bool copy) {
    return Array::ToDVArray(Cast(ad), copy);
  }
  static ArrayData* ToHackArr(ArrayData* ad, bool copy) {
    return Array::ToHackArr(Cast(ad), copy);
  }
  static ArrayData* SetLegacyArray(ArrayData* ad, bool copy, bool legacy) {
    return Array::SetLegacyArray(Cast(ad), copy, legacy);
  }
};


template <typename Array>
constexpr LayoutFunctions fromArray() {
  LayoutFunctions result;
  if constexpr (debug) {
#define X(Return, Name, Args...) \
  result.fn##Name = LayoutFunctionDispatcher<Array>::Name;
  BESPOKE_LAYOUT_FUNCTIONS(ArrayData)
#undef X
  } else {
#define X(Return, Name, Args...) \
  result.fn##Name = reinterpret_cast<Return(*)(Args)>(Array::Name);
  BESPOKE_LAYOUT_FUNCTIONS(ArrayData)
#undef X
  }
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
  Layout(LayoutIndex index, const std::string& description,
         const LayoutFunctions* vtable = nullptr);
  virtual ~Layout() {}

  /*
   * Bespoke indexes are 15 bits wide. When we store them in m_extra of
   * ArrayData, we always set the sign bit, which allows us to test that
   * (m_size >= constant && isVanilla()) in a single comparison.
   */
  static constexpr LayoutIndex kMaxIndex = {(1 << 15) - 1};

  LayoutIndex index() const { return m_index; }
  const std::string& describe() const { return m_description; }
  const LayoutFunctions* vtable() const { return m_vtable; }

  /*
   * In order to support efficient layout type tests in the JIT, we let
   * layout initializers reserve aligned blocks of indices to populate.
   *
   *   @precondition:  `size` must be a power of two.
   *   @postcondition: The result is a multiple of size.
   */
  static LayoutIndex ReserveIndices(size_t size);

  static const Layout* FromIndex(LayoutIndex index);

  ///////////////////////////////////////////////////////////////////////////

  /*
   * JIT support
   *
   * In all the irgen emit helpers below, `arr` is guaranteed to be an array
   * matching this BespokeLayout's type class.
   *
   * For those methods that take `key`, it is guaranteed to be a valid key
   * for the base's type. For example, if `arr` is a dict, then `key` is an
   * arraykey, and if `arr` is a vec, `key` is an int. (We make no claims
   * about whether `key` matches tighter per-layout type constraints.)
   */

  using SSATmp = jit::SSATmp;
  using Block = jit::Block;
  using IRGS = jit::irgen::IRGS;

  /*
   * Return the value at `key` in `arr`, branching to `taken` if the key is
   * not present. This operation does no refcounting.
   */
  virtual SSATmp* emitGet(
      IRGS& env, SSATmp* arr, SSATmp* key, Block* taken) const;

  /*
   * Create a new array by setting `arr[key] = val`, CoWing or escalating as
   * needed. This op consumes a ref on `arr` and produces a ref on the result.
   */
  virtual SSATmp* emitSet(
      IRGS& env, SSATmp* arr, SSATmp* key, SSATmp* val) const;

  /*
   * Create a new array by setting `arr[] = val`, CoWing or escalating as
   * needed. This op consumes a ref on `arr` and produces a ref on the result.
   */
  virtual SSATmp* emitAppend(
    IRGS& env, SSATmp* arr, SSATmp* val) const;

private:
  struct Initializer;
  static Initializer s_initializer;

  LayoutIndex m_index;
  std::string m_description;
  const LayoutFunctions* m_vtable;
};

}}

#endif // HPHP_BESPOKEDIR_LAYOUT_H_
