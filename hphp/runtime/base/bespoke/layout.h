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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/req-tiny-vector.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/jit/array-layout.h"
#include "hphp/util/type-scan.h"

#include <set>

namespace HPHP::bespoke {

// Although we dynamically construct bespoke layouts, we only have a small,
// statically-known list of "families" of these layouts.
//
// We restrict layout indices: the upper byte of a bespoke layout's index must
// match the "layout byte" for its layout family. That means that we're limited
// to 256 layouts for a given family.
//
// Struct dicts have multiple valid layout bytes. Any layout byte with bit 0
// (the lowest bit) set and bit 7 (the highest bit) unset is a valid struct
// layout byte, allowing us to create 64 * 256 struct layouts.
//
// This restriction helps us in two ways:
//
//   1. It lets us do bespoke vtable dispatch off this byte alone.
//
//   2. It lets us choose indices that we can efficiently test for. All layout
//      tests are a single "test" op. See also "unordered code" in datatype.h.
//
// These constants look ad-hoc. Here's what the bits mean:
//  - Bit 1: unset iff subtype of MonotypeVec<Top>
//  - Bit 2: unset iff subtype of MonotypeDict<Empty|Int,Top>
//  - Bit 3: unset iff subtype of MonotypeDict<Empty|Str,Top>
//
// Bit 4 is less constrained. For MonotypeDict, when it is unset, it means the
// layout is one of the static-string keyed layouts. For MonotypeVec, when it
// is unset, it means the layout is the empty singleton.
//
constexpr uint8_t kLoggingLayoutByte               = 0b10001110;
constexpr uint8_t kMonotypeVecLayoutByte           = 0b10011100;
constexpr uint8_t kEmptyMonotypeVecLayoutByte      = 0b10001100;
constexpr uint8_t kIntMonotypeDictLayoutByte       = 0b10011010;
constexpr uint8_t kStrMonotypeDictLayoutByte       = 0b10010110;
constexpr uint8_t kStaticStrMonotypeDictLayoutByte = 0b10000110;
constexpr uint8_t kEmptyMonotypeDictLayoutByte     = 0b10000010;
constexpr uint8_t kTypeStructureLayoutByte         = 0b10011110;
constexpr uint8_t kBespokeVtableMask               = 0b00011111;

// Log that we're calling the given function for the given array.
void logBespokeDispatch(const BespokeArray* bad, const char* fn);

// Return a monotype copy of a vanilla array, or nullptr if it's not monotype.
BespokeArray* maybeMonoify(ArrayData*);

// Return a struct copy of a vanilla array, or nullptr if it's not struct-like.
BespokeArray* maybeStructify(ArrayData* ad, const LoggingProfile* profile);

#define BESPOKE_LAYOUT_FUNCTIONS(T) \
  X(void, Scan, const T* ad, type_scan::Scanner& scanner) \
  X(ArrayData*, EscalateToVanilla, const T*, const char* reason) \
  X(void, ConvertToUncounted, T*, const MakeUncountedEnv& env) \
  X(void, ReleaseUncounted, T*) \
  X(void, Release, T*) \
  X(bool, IsVectorData, const T*) \
  X(TypedValue, NvGetInt, const T*, int64_t) \
  X(TypedValue, NvGetStr, const T*, const StringData*) \
  X(TypedValue, GetPosKey, const T*, ssize_t pos) \
  X(TypedValue, GetPosVal, const T*, ssize_t pos) \
  X(bool, PosIsValid, const T*, ssize_t pos) \
  X(ssize_t, IterBegin, const T*) \
  X(ssize_t, IterLast, const T*) \
  X(ssize_t, IterEnd, const T*) \
  X(ssize_t, IterAdvance, const T*, ssize_t) \
  X(ssize_t, IterRewind, const T*, ssize_t) \
  X(arr_lval, LvalInt, T* ad, int64_t k) \
  X(arr_lval, LvalStr, T* ad, StringData* k) \
  X(tv_lval, ElemInt, tv_lval lval, int64_t k, bool) \
  X(tv_lval, ElemStr, tv_lval lval, StringData* k, bool) \
  X(ArrayData*, SetIntMove, T*, int64_t k, TypedValue v) \
  X(ArrayData*, SetStrMove, T*, StringData* k, TypedValue v)\
  X(ArrayData*, RemoveIntMove, T*, int64_t) \
  X(ArrayData*, RemoveStrMove, T*, const StringData*) \
  X(ArrayData*, AppendMove, T*, TypedValue v) \
  X(ArrayData*, PopMove, T*, Variant&) \
  X(ArrayData*, PreSort, T*, SortFunction sf) \
  X(ArrayData*, PostSort, T*, ArrayData* vad) \
  X(ArrayData*, SetLegacyArray, T*, bool copy, bool legacy) \
  X(ArrayData*, Copy, const T*)

#define BESPOKE_SYNTHESIZED_LAYOUT_FUNCTIONS(T) \
  X(TypedValue, NvGetIntThrow, const T*, int64_t) \
  X(TypedValue, NvGetStrThrow, const T*, const StringData*)

struct LayoutFunctions {
#define X(Return, Name, Args...) Return (*fn##Name)(Args);
  BESPOKE_LAYOUT_FUNCTIONS(ArrayData)
  BESPOKE_SYNTHESIZED_LAYOUT_FUNCTIONS(ArrayData)
#undef X
};

struct LayoutFunctionsDispatch {
#define X(Return, Name, Args...) \
  Return (*fn##Name[kBespokeVtableMask + 1])(Args);
  BESPOKE_LAYOUT_FUNCTIONS(ArrayData)
  BESPOKE_SYNTHESIZED_LAYOUT_FUNCTIONS(ArrayData)
#undef X
};

extern LayoutFunctionsDispatch g_layout_funcs;

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
  using SynthFuncs = SynthesizedArrayFunctions<Array>;

  ALWAYS_INLINE static Array* Cast(ArrayData* ad, const char* fn) {
    logBespokeDispatch(BespokeArray::asBespoke(ad), fn);
    return Array::As(ad);
  }
  ALWAYS_INLINE static const Array* Cast(const ArrayData* ad, const char* fn) {
    logBespokeDispatch(BespokeArray::asBespoke(ad), fn);
    return Array::As(ad);
  }

  static void Scan(const ArrayData* ad, type_scan::Scanner& scanner) {
    return Array::Scan(Cast(ad, __func__), scanner);
  }
  static ArrayData* EscalateToVanilla(const ArrayData* ad, const char* reason) {
    return Array::EscalateToVanilla(Cast(ad, __func__), reason);
  }
  static void ConvertToUncounted(ArrayData* ad, const MakeUncountedEnv& env) {
    return Array::ConvertToUncounted(Cast(ad, __func__), env);
  }
  static void ReleaseUncounted(ArrayData* ad) {
    return Array::ReleaseUncounted(Cast(ad, __func__));
  }
  static void Release(ArrayData* ad) {
    return Array::Release(Cast(ad, __func__));
  }
  static bool IsVectorData(const ArrayData* ad) {
    return Array::IsVectorData(Cast(ad, __func__));
  }
  static TypedValue NvGetInt(const ArrayData* ad, int64_t k) {
    return Array::NvGetInt(Cast(ad, __func__), k);
  }
  static TypedValue NvGetStr(const ArrayData* ad, const StringData* k) {
    return Array::NvGetStr(Cast(ad, __func__), k);
  }
  static TypedValue NvGetIntThrow(const ArrayData* ad, int64_t k) {
    return SynthFuncs::NvGetIntThrow(Cast(ad, __func__), k);
  }
  static TypedValue NvGetStrThrow(const ArrayData* ad, const StringData* k) {
    return SynthFuncs::NvGetStrThrow(Cast(ad, __func__), k);
  }
  static TypedValue GetPosKey(const ArrayData* ad, ssize_t pos) {
    return Array::GetPosKey(Cast(ad, __func__), pos);
  }
  static TypedValue GetPosVal(const ArrayData* ad, ssize_t pos) {
    return Array::GetPosVal(Cast(ad, __func__), pos);
  }
  static bool PosIsValid(const ArrayData* ad, ssize_t pos) {
    return Array::PosIsValid(Cast(ad, __func__), pos);
  }
  static arr_lval LvalInt(ArrayData* ad, int64_t k) {
    return Array::LvalInt(Cast(ad, __func__), k);
  }
  static arr_lval LvalStr(ArrayData* ad, StringData* k) {
    return Array::LvalStr(Cast(ad, __func__), k);
  }
  static tv_lval ElemInt(tv_lval lval, int64_t k, bool throwOnMissing) {
    Cast(lval.val().parr, __func__);
    return Array::ElemInt(lval, k, throwOnMissing);
  }
  static tv_lval ElemStr(tv_lval lval, StringData* k, bool throwOnMissing) {
    Cast(lval.val().parr, __func__);
    return Array::ElemStr(lval, k, throwOnMissing);
  }
  static ArrayData* SetIntMove(ArrayData* ad, int64_t k, TypedValue v) {
    assertx(type(v) != KindOfUninit);
    return Array::SetIntMove(Cast(ad, __func__), k, v);
  }
  static ArrayData* SetStrMove(ArrayData* ad, StringData* k, TypedValue v){
    assertx(type(v) != KindOfUninit);
    return Array::SetStrMove(Cast(ad, __func__), k, v);
  }
  static ArrayData* RemoveIntMove(ArrayData* ad, int64_t k) {
    return Array::RemoveIntMove(Cast(ad, __func__), k);
  }
  static ArrayData* RemoveStrMove(ArrayData* ad, const StringData* k) {
    return Array::RemoveStrMove(Cast(ad, __func__), k);
  }
  static ssize_t IterBegin(const ArrayData* ad) {
    return Array::IterBegin(Cast(ad, __func__));
  }
  static ssize_t IterLast(const ArrayData* ad) {
    return Array::IterLast(Cast(ad, __func__));
  }
  static ssize_t IterEnd(const ArrayData* ad) {
    return Array::IterEnd(Cast(ad, __func__));
  }
  static ssize_t IterAdvance(const ArrayData* ad, ssize_t pos) {
    return Array::IterAdvance(Cast(ad, __func__), pos);
  }
  static ssize_t IterRewind(const ArrayData* ad, ssize_t pos) {
    return Array::IterRewind(Cast(ad, __func__), pos);
  }
  static ArrayData* AppendMove(ArrayData* ad, TypedValue v) {
    assertx(type(v) != KindOfUninit);
    return Array::AppendMove(Cast(ad, __func__), v);
  }
  static ArrayData* PopMove(ArrayData* ad, Variant& v) {
    return Array::PopMove(Cast(ad, __func__), v);
  }
  static ArrayData* PreSort(ArrayData* ad, SortFunction sf) {
    return Array::PreSort(Cast(ad, __func__), sf);
  }
  static ArrayData* PostSort(ArrayData* ad, ArrayData* vad) {
    return Array::PostSort(Cast(ad, __func__), vad);
  }
  static ArrayData* SetLegacyArray(ArrayData* ad, bool copy, bool legacy) {
    return Array::SetLegacyArray(Cast(ad, __func__), copy, legacy);
  }
  static ArrayData* Copy(const ArrayData* ad) {
    return Array::Copy(Cast(ad, __func__));
  }
};

template <typename Array>
constexpr LayoutFunctions fromArray() {
  LayoutFunctions result;
  if constexpr (debug) {
#define X(Return, Name, Args...) \
    result.fn##Name = LayoutFunctionDispatcher<Array>::Name;
    BESPOKE_LAYOUT_FUNCTIONS(ArrayData)
    BESPOKE_SYNTHESIZED_LAYOUT_FUNCTIONS(ArrayData)
#undef X
  } else {
#define X(Return, Name, Args...) \
    result.fn##Name = reinterpret_cast<Return(*)(Args)>(Array::Name);
    BESPOKE_LAYOUT_FUNCTIONS(ArrayData)
#undef X
#define X(Return, Name, Args...) \
    { \
      auto const fn = SynthesizedArrayFunctions<Array>::Name; \
      result.fn##Name = reinterpret_cast<Return(*)(Args)>(fn); \
    }
    BESPOKE_SYNTHESIZED_LAYOUT_FUNCTIONS(ArrayData)
#undef X
  }
  return result;
}

/*
 * A bespoke::Layout can represent either both the concrete layout of a given
 * BespokeArray or some abstract type that's a union of concrete layouts.
 *
 * bespoke::Layout* also forms a type lattice. BespokeTop is the top type, and
 * the null layout is the bottom type. We construct this lattice incrementally:
 * a layout must declare edges to its (pre-existing) parents on construction.
 * This constraint means that layout creation order is a topological sort.
 *
 * (NOTE: Parent edges do not need to form a covering relation on the lattice.
 * The set of all ancestors of a given layout is defined by the transitive
 * closure of the parent edges.)
 *
 * Once the type hierarchy has been created, we supply the standard <=, meet
 * (&), and join (|) operations for the layouts, which are used from ArraySpec
 * for type operations.
 *
 * When the layout hierarchy is final, all type operations are valid. Before
 * the layout hierarchy is final, only type operations on BespokeTop are
 * permitted. This enables us to use BespokeTop and normal type operations in
 * profiling tracelets while disallowing more specific type operations that
 * require knowledge of the bespoke hierarchy.
 */
struct Layout {
  using LayoutSet = std::set<LayoutIndex>;

  Layout(const Layout& l) = delete;
  Layout& operator=(const Layout&) = delete;

  virtual ~Layout() {}

  /*
   * Bespoke indexes are 15 bits wide. When we store them in m_extra of
   * ArrayData, we always set the sign bit, which allows us to test that
   * (m_size >= constant && isVanilla()) in a single comparison.
   */
  static constexpr LayoutIndex kMaxIndex = {(1 << 15) - 1};

  LayoutIndex index() const { return m_index; }
  const std::string& describe() const { return m_description; }
  std::string dumpInformation() const;
  virtual bool isConcrete() const { return false; }
  const LayoutFunctions* vtable() const { return m_vtable; }
  LayoutTest getLayoutTest() const;

  /*
   * Coerce a vanilla array to have this layout. Returns nullptr on failure.
   * This method is PRc. It does not consume a refcount on the input.
   *
   * This method does not work on LoggingLayout. Use one of the LoggingArray
   * constructors directly, providing a profile, to make a LoggingArray.
   */
  BespokeArray* coerce(ArrayData* ad) const;

  /*
   * Access to individual layouts, or debug access to all of them.
   */
  static const Layout* FromIndex(LayoutIndex index);
  static std::string dumpAllLayouts();

  /*
   * Test-only helper to clear the existing layouts in the type hierarchy.
   * After calling this method, the only layout will be the BespokeTop one.
   */
  static void ClearHierarchy();

  /*
   * Seals the bespoke type hierarchy. Before this is invoked, type operations
   * on bespoke layouts other than BespokeTop are invalid. After it is invoked,
   * all type operations are valid but no new layouts can be created.
   */
  static void FinalizeHierarchy();

  /*
   * Returns true if we've finalized layout decisions and sealed the bespoke
   * type lattice. (The latter must happen after layout selection.)
   */
  static bool HierarchyFinalized();

  bool operator<=(const Layout& other) const;
  const Layout* operator|(const Layout& other) const;
  const Layout* operator&(const Layout& other) const;

  ///////////////////////////////////////////////////////////////////////////

  using ArrayLayout = jit::ArrayLayout;
  using Type = jit::Type;

  /*
   * Returns the most specific layout known for the result of appending a val
   * of type `val` to an array with this layout.
   */
  virtual ArrayLayout appendType(Type val) const;

  /*
   * Returns the most specific layout known for the result of removing a key
   * of type `key` from an array with this layout.
   */
  virtual ArrayLayout removeType(Type key) const;

  /*
   * Returns the most specific layout known for the result of setting a key
   * of type `key` to a val of type `val` for an array with this layout.
   */
  virtual ArrayLayout setType(Type key, Type val) const;

  /*
   * Returns the most specific type known for the element at the given key for
   * this bespoke layout. The pair returned contains this type, along with a
   * boolean indicating if element is statically known to be present.
   */
  virtual std::pair<Type, bool> elemType(Type key) const;

  /*
   * Returns the type bound for a slot of a struct dict (returns TCell
   * for non-struct dicts). If the slot is not known specifically, the
   * union of all possible slots is returned.
   */
  virtual Type getTypeBound(Type slot) const;

  /*
   * Returns true if the given slot of a struct dict will always have
   * a value present. Note: this does not check if the slot is
   * actually present. (IE: if you pass just TInt as the slot, and all
   * slots in the layout as required, it will return true).
   */
  virtual bool slotAlwaysPresent(const Type& slot) const;

  /*
   * Returns the most specific type known for the first or last key or value
   * for this bespoke layout. The pair returned contains this type, along with
   * a boolean indicating if key/value is statically known to be present.
   */
  virtual std::pair<Type, bool> firstLastType(bool isFirst, bool isKey) const;

  /*
   * Returns the most specific type known for the key or value at the specified
   * iterator position for this bespoke layout. The iterator position can be
   * assumed to be valid.
   */
  virtual Type iterPosType(Type pos, bool isKey) const;

  /*
   * If this layout contains a definite number of elements, return
   * that number.
   */
  virtual Optional<int64_t> numElements() const;

  ///////////////////////////////////////////////////////////////////////////

protected:
  Layout(LayoutIndex index, std::string description, LayoutSet parents,
         const LayoutFunctions* vtable);

private:
  bool checkInvariants() const;
  LayoutSet computeAncestors() const;
  LayoutSet computeDescendants() const;
  LayoutTest computeLayoutTest() const;

  bool isDescendantOfDebug(const Layout* other) const;

  struct Initializer;
  static Initializer s_initializer;
  struct BFSWalker;

  struct DescendantOrdering;
  struct AncestorOrdering;

  LayoutIndex m_index;
  size_t m_topoIndex;
  std::string m_description;
  LayoutSet m_parents;
  LayoutSet m_children;
  std::vector<Layout*> m_descendants;
  std::vector<Layout*> m_ancestors;
  LayoutTest m_layout_test;

protected:
  const LayoutFunctions* m_vtable;
};

/*
 * An abstract bespoke layout, providing precious little on top of Layout.
 */
struct AbstractLayout : public Layout {
  AbstractLayout(LayoutIndex index, std::string description, LayoutSet parents,
                 const LayoutFunctions* vtable = nullptr);
  virtual ~AbstractLayout() {}

  static void InitializeLayouts();

  static LayoutIndex GetBespokeTopIndex();
};

/*
 * A concrete bespoke layout providing a vtable to access the bespoke array
 * implementation methods. It also provides default implementations for the
 * various JIT helpers in terms of the vtable methods.
 */
struct ConcreteLayout : public Layout {
  ConcreteLayout(LayoutIndex index, std::string description,
                 LayoutSet parents, const LayoutFunctions* vtable);
  virtual ~ConcreteLayout() {}

  bool isConcrete() const override { return true; }

  static const ConcreteLayout* FromConcreteIndex(LayoutIndex index);
};

// Global view, used for debugging and serialization.
void eachLayout(std::function<void(Layout& layout)> fn);

// Array of all layout pointers. Useful for accessing from the TC.
Layout** layoutsForJIT();

// For logging.
size_t numStructLayouts();

}
