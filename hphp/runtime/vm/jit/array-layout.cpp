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

#include "hphp/runtime/vm/jit/array-layout.h"

#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////////////

namespace {

using Sort = ArrayLayout::Sort;

auto constexpr kBasicSortMask    = 0b11;
auto constexpr kBasicSortShift   = 0b11;
auto constexpr kBasicSortUnshift = 0b01;

// A "basic sort" is just one of the four named Sort enum values. If `sort`
// is non-basic, then Sort::Bottom < sort < Sort::Bespoke.
constexpr bool isBasicSort(Sort sort) {
  return sort <= Sort::Bespoke;
}

// Converts non-basic sorts (which are subtypes of Bespoke) to Bespoke.
constexpr Sort toBasicSort(Sort sort) {
  return std::min(sort, Sort::Bespoke);
}

// If we mask a basic sort, we'll get a value such that | and & bit ops on
// that value correspond to | and & type operations on the original sort.
constexpr int maskBasicSort(Sort sort) {
  assertx(isBasicSort(sort));
  return kBasicSortMask & (int(sort) + kBasicSortShift);
}

static_assert(maskBasicSort(Sort::Top)     == 0b11);
static_assert(maskBasicSort(Sort::Vanilla) == 0b01);
static_assert(maskBasicSort(Sort::Bespoke) == 0b10);
static_assert(maskBasicSort(Sort::Bottom)  == 0b00);

// This operation is the inverse of the maskBasicSort operation above.
constexpr Sort unmaskBasicSort(int masked) {
  auto const result = Sort(kBasicSortMask & (masked + kBasicSortUnshift));
  assertx(isBasicSort(result));
  return result;
}

static_assert(unmaskBasicSort(maskBasicSort(Sort::Top))     == Sort::Top);
static_assert(unmaskBasicSort(maskBasicSort(Sort::Vanilla)) == Sort::Vanilla);
static_assert(unmaskBasicSort(maskBasicSort(Sort::Bespoke)) == Sort::Bespoke);
static_assert(unmaskBasicSort(maskBasicSort(Sort::Bottom))  == Sort::Bottom);

// Returns the basic sort that is the intersection of the given basic sorts.
constexpr Sort intersectBasicSort(Sort a, Sort b) {
  return unmaskBasicSort(maskBasicSort(a) & maskBasicSort(b));
}

// Returns the basic sort that is the union of the given basic sorts.
constexpr Sort unionBasicSort(Sort a, Sort b) {
  return unmaskBasicSort(maskBasicSort(a) | maskBasicSort(b));
}

// Returns the sort (either Bespoke, or non-basic) for this bespoke layout.
Sort sortFromLayoutIndex(bespoke::LayoutIndex index) {
  return Sort(index.raw + int(Sort::Bespoke));
}

const bespoke::Layout& assertBespoke(ArrayLayout layout) {
  auto const result = layout.bespokeLayout();
  assertx(result != nullptr);
  return *result;
}

}

//////////////////////////////////////////////////////////////////////////////

ArrayLayout::ArrayLayout(bespoke::LayoutIndex index)
  : sort(sortFromLayoutIndex(index))
{
  assertx(bespoke::Layout::FromIndex(*layoutIndex()));
}

ArrayLayout::ArrayLayout(const bespoke::Layout* layout)
  : sort(sortFromLayoutIndex(layout->index()))
{
  assertx(bespoke::Layout::FromIndex(*layoutIndex()));
}

bool ArrayLayout::operator<=(const ArrayLayout& o) const {
  if (*this == o) return true;
  if (o == Top()) return true;
  if (*this == Bottom()) return true;

  // The max chain length on basic sorts alone is three:
  //
  //   Bottom < {Vanilla,Bespoke} < Top
  //
  // We took care of the Bottom, Top, and equality cases above. Further, if o
  // is non-basic, it's a strict subtype of Bespoke. So we can return here.
  if (isBasicSort(sort)) return false;

  if (isBasicSort(o.sort)) return o == Bespoke();
  return assertBespoke(*this) <= assertBespoke(o);
}

ArrayLayout ArrayLayout::operator|(const ArrayLayout& o) const {
  if (*this == o) return o;
  if (o == Bottom()) return *this;
  if (*this == Bottom()) return o;

  // If either side is captured as a basic sort, then the result is, too.
  if (isBasicSort(sort) || isBasicSort(o.sort)) {
    return ArrayLayout(unionBasicSort(toBasicSort(sort), toBasicSort(o.sort)));
  }

  return ArrayLayout(assertBespoke(*this) | assertBespoke(o));
}

ArrayLayout ArrayLayout::operator&(const ArrayLayout& o) const {
  if (*this == o) return o;
  if (o == Top()) return *this;
  if (*this == Top()) return o;

  // We only intersect bespoke layouts if toBasicSort is Bespoke for both.
  auto const meet = intersectBasicSort(toBasicSort(sort), toBasicSort(o.sort));
  if (meet != Sort::Bespoke) return ArrayLayout(meet);

  // If either type is Bespoke (i.e. "bespoke top"), return the other type.
  if (o == Bespoke()) return *this;
  if (*this == Bespoke()) return o;
  auto const result = assertBespoke(*this) & assertBespoke(o);
  return result ? ArrayLayout(result) : Bottom();
}

const bespoke::Layout* ArrayLayout::bespokeLayout() const {
  auto const index = int(sort) - int(Sort::Bespoke);
  if (index < 0) return nullptr;
  return bespoke::Layout::FromIndex({safe_cast<uint16_t>(index)});
}

const bespoke::ConcreteLayout* ArrayLayout::concreteLayout() const {
  auto const index = int(sort) - int(Sort::Bespoke);
  if (index < 0) return nullptr;
  auto const layout = bespoke::Layout::FromIndex({safe_cast<uint16_t>(index)});
  auto const result = layout->isConcrete()
    ? reinterpret_cast<const bespoke::ConcreteLayout*>(layout)
    : nullptr;
  assertx(result == dynamic_cast<const bespoke::ConcreteLayout*>(layout));
  return result;
}

folly::Optional<bespoke::LayoutIndex> ArrayLayout::layoutIndex() const {
  auto const index = int(sort) - int(Sort::Bespoke);
  if (index < 0) return {};
  return bespoke::LayoutIndex { safe_cast<uint16_t>(index) };
}

const bespoke::Layout* ArrayLayout::irgenLayout() const {
  auto const index = std::max(int(sort) - int(Sort::Bespoke), 0);
  return bespoke::Layout::FromIndex({safe_cast<uint16_t>(index)});
}

std::string ArrayLayout::describe() const {
  if (isBasicSort(sort)) {
    switch (sort) {
      case Sort::Top:     return "Top";
      case Sort::Vanilla: return "Vanilla";
      case Sort::Bespoke: return "Bespoke";
      case Sort::Bottom:  return "Bottom";
    }
  }
  return folly::sformat("Bespoke({})", assertBespoke(*this).describe());
}

//////////////////////////////////////////////////////////////////////////////

namespace {
bool checkLayoutMatches(const ArrayLayout& layout, SSATmp* arr) {
  auto const spec = arr->type().arrSpec();
  DEBUG_ONLY auto const converted = [&]{
    if (spec == ArraySpec::Bottom()) return ArrayLayout::Bottom();
    if (spec.vanilla())              return ArrayLayout::Vanilla();
    if (auto const l = spec.bespokeLayout()) {
      return ArrayLayout(l->index());
    }
    return ArrayLayout::Top();
  }();
  assertx(converted <= layout);
  return true;
}
}

SSATmp* ArrayLayout::emitGet(
    IRGS& env, SSATmp* arr, SSATmp* key, Block* taken) const {
  assertx(checkLayoutMatches(*this, arr));
  return irgenLayout()->emitGet(env, arr, key, taken);
}

SSATmp* ArrayLayout::emitElem(
    IRGS& env, SSATmp* arr, SSATmp* key, bool throwOnMissing) const {
  assertx(checkLayoutMatches(*this, arr));
  return irgenLayout()->emitElem(env, arr, key, throwOnMissing);
}

SSATmp* ArrayLayout::emitSet(
    IRGS& env, SSATmp* arr, SSATmp* key, SSATmp* val) const {
  assertx(checkLayoutMatches(*this, arr));
  return irgenLayout()->emitSet(env, arr, key, val);
}

SSATmp* ArrayLayout::emitAppend(IRGS& env, SSATmp* arr, SSATmp* val) const {
  assertx(checkLayoutMatches(*this, arr));
  return irgenLayout()->emitAppend(env, arr, val);
}

SSATmp* ArrayLayout::emitEscalateToVanilla(
    IRGS& env, SSATmp* arr, const char* reason) const {
  assertx(checkLayoutMatches(*this, arr));
  return irgenLayout()->emitEscalateToVanilla(env, arr, reason);
}

SSATmp* ArrayLayout::emitIterFirstPos(IRGS& env, SSATmp* arr) const {
  assertx(checkLayoutMatches(*this, arr));
  return irgenLayout()->emitIterFirstPos(env, arr);
}

SSATmp* ArrayLayout::emitIterLastPos(IRGS& env, SSATmp* arr) const {
  assertx(checkLayoutMatches(*this, arr));
  return irgenLayout()->emitIterLastPos(env, arr);
}

SSATmp* ArrayLayout::emitIterPos(IRGS& env, SSATmp* arr, SSATmp* idx) const {
  assertx(checkLayoutMatches(*this, arr));
  return irgenLayout()->emitIterPos(env, arr, idx);
}

SSATmp* ArrayLayout::emitIterElm(IRGS& env, SSATmp* arr, SSATmp* pos) const {
  assertx(checkLayoutMatches(*this, arr));
  return irgenLayout()->emitIterElm(env, arr, pos);
}

SSATmp* ArrayLayout::emitIterGetKey(IRGS& env, SSATmp* arr, SSATmp* elm) const {
  assertx(checkLayoutMatches(*this, arr));
  return irgenLayout()->emitIterGetKey(env, arr, elm);
}

SSATmp* ArrayLayout::emitIterGetVal(IRGS& env, SSATmp* arr, SSATmp* elm) const {
  assertx(checkLayoutMatches(*this, arr));
  return irgenLayout()->emitIterGetVal(env, arr, elm);
}

//////////////////////////////////////////////////////////////////////////////

}}
