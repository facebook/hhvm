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
#include <string>

#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/req-tiny-vector.h"

#include <folly/Optional.h>

namespace HPHP { namespace jit {

struct Block;
struct SSATmp;

namespace irgen { struct IRGS; }

/*
 * A small lattice on array layouts, combining both vanilla array layouts
 * (PackedArray, MixedArray, SetArray) and bespoke array layouts. This type
 * is kept small (just 2 bytes) and operations are implemented efficiently
 * so that it can be incorporated as part of the JIT's ArraySpec.
 */
struct ArrayLayout {
  explicit ArrayLayout(bespoke::LayoutIndex layout);
  explicit ArrayLayout(const bespoke::Layout* layout);

  bool operator==(const ArrayLayout& o) const { return o.sort == sort; }
  bool operator!=(const ArrayLayout& o) const { return o.sort != sort; }

  // Type-system operations on ArrayLayout: subtype, union, intersection.
  bool operator<=(const ArrayLayout& o) const;
  ArrayLayout operator|(const ArrayLayout& o) const;
  ArrayLayout operator&(const ArrayLayout& o) const;

  // Returns true if the layout is definitely {vanilla,bespoke}.
  bool vanilla() const { return sort == Sort::Vanilla; }
  bool bespoke() const { return sort >= Sort::Bespoke; }

  // The result is non-null iff the layout is a (concrete) bespoke layout.
  const bespoke::Layout* bespokeLayout() const;
  const bespoke::ConcreteLayout* concreteLayout() const;
  folly::Optional<bespoke::LayoutIndex> layoutIndex() const;

  req::TinyVector<MaskAndCompare, 2> bespokeMaskAndCompareSet() const;

  // Return a human-readable debug string describing the layout.
  std::string describe() const;

  // Simple "serialization" allowing us to pack this struct in ArraySpec.
  constexpr uint16_t toUint16() const { return uint16_t(sort); }

  /**************************************************************************
   * Static helpers to get basic layouts
   **************************************************************************/

  static constexpr ArrayLayout Top()     { return ArrayLayout(Sort::Top); }
  static constexpr ArrayLayout Vanilla() { return ArrayLayout(Sort::Vanilla); }
  static constexpr ArrayLayout Bespoke() { return ArrayLayout(Sort::Bespoke); }
  static constexpr ArrayLayout Bottom()  { return ArrayLayout(Sort::Bottom); }

  static constexpr ArrayLayout FromUint16(uint16_t x) {
    return ArrayLayout(Sort(x));
  }

  /**************************************************************************
   * JIT support at the irgen level
   **************************************************************************/

  using IRGS = irgen::IRGS;

  SSATmp* emitGet(IRGS& env, SSATmp* arr, SSATmp* key, Block* taken) const;
  SSATmp* emitElem(IRGS& env, SSATmp* arr, SSATmp* key, bool throwOnMissing) const;
  SSATmp* emitSet(IRGS& env, SSATmp* arr, SSATmp* key, SSATmp* val) const;
  SSATmp* emitAppend(IRGS& env, SSATmp* arr, SSATmp* val) const;
  SSATmp* emitEscalateToVanilla(IRGS& env, SSATmp* arr, const char* reason) const;
  SSATmp* emitIterFirstPos(IRGS& env, SSATmp* arr) const;
  SSATmp* emitIterLastPos(IRGS& env, SSATmp* arr) const;
  SSATmp* emitIterPos(IRGS& env, SSATmp* arr, SSATmp* idx) const;
  SSATmp* emitIterElm(IRGS& env, SSATmp* arr, SSATmp* pos) const;
  SSATmp* emitIterGetKey(IRGS& env, SSATmp* arr, SSATmp* elm) const;
  SSATmp* emitIterGetVal(IRGS& env, SSATmp* arr, SSATmp* elm) const;

  /**************************************************************************
   * Representation as a simple uint16_t
   **************************************************************************/

  // sort may be greater than Sort::Bespoke. If sort >= Sort::Bespoke, then
  // it maps to a bespoke::Layout with ID equal to sort - Sort::Bespoke.
  //
  // (In particular, we require that the Top bespoke::Layout has ID 0.)
  //
  // These sorts are arranged so that we can efficiently do type operations
  // | and & on them - see logic around "isBasicSort" in the implementation.
  enum class Sort : uint16_t { Top, Bottom, Vanilla, Bespoke };

private:
  constexpr explicit ArrayLayout(Sort sort) : sort(sort) {}

  // Returns a bespoke::Layout that can be used to JIT code for this layout.
  // We use the fact that BespokeTop codegen can also handle vanilla arrays.
  const bespoke::Layout* irgenLayout() const;

  Sort sort;
};

}}
