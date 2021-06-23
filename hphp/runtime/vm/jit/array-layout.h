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

#include "hphp/util/optional.h"

namespace HPHP { namespace jit {

struct Block;
struct SSATmp;
struct ProfDataSerializer;
struct ProfDataDeserializer;
struct Type;

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

  // Test for specific kinds of bespoke layouts.
  bool logging() const;
  bool monotype() const;
  bool is_struct() const;
  bool is_concrete() const;

  // The result is non-null iff the layout is a bespoke layout.
  const bespoke::Layout* bespokeLayout() const;
  Optional<bespoke::LayoutIndex> layoutIndex() const;

  LayoutTest bespokeLayoutTest() const;

  // Return a human-readable debug string describing the layout.
  std::string describe() const;

  // Serialization support, used to pack ArrayLayout in binary formats.
  constexpr uint16_t toUint16() const { return uint16_t(sort); }

  // "apply" applies this layout to a static array, so we can deserialize it.
  // Careful: serialization does NOT round-trip logging arrays.
  ArrayData* apply(ArrayData* ad) const;

  /**************************************************************************
   * Static helpers to get basic layouts
   **************************************************************************/

  static constexpr ArrayLayout Top()     noexcept { return ArrayLayout(Sort::Top); }
  static constexpr ArrayLayout Vanilla() noexcept { return ArrayLayout(Sort::Vanilla); }
  static constexpr ArrayLayout Bespoke() noexcept { return ArrayLayout(Sort::Bespoke); }
  static constexpr ArrayLayout Bottom()  noexcept { return ArrayLayout(Sort::Bottom); }

  static constexpr ArrayLayout FromUint16(uint16_t x) {
    return ArrayLayout(Sort(x));
  }

  static ArrayLayout FromArray(const ArrayData* ad) {
    if (ad->isVanilla()) return Vanilla();
    return ArrayLayout(BespokeArray::asBespoke(ad)->layoutIndex());
  }

  /**************************************************************************
   * Type inference functions
   **************************************************************************/

  // The array's new layout after a mutating operation.
  ArrayLayout appendType(Type val) const;
  ArrayLayout removeType(Type key) const;
  ArrayLayout setType(Type key, Type val) const;

  // The second element of the pair is true if the key is definitely present.
  std::pair<Type, bool> elemType(Type key) const;
  std::pair<Type, bool> firstLastType(bool isFirst, bool isKey) const;

  Type iterPosType(Type pos, bool isKey) const;

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
  constexpr explicit ArrayLayout(Sort sort) noexcept : sort(sort) {}

  // Returns a bespoke::Layout that can be used to JIT code for this layout.
  // We use the fact that BespokeTop codegen can also handle vanilla arrays.
  const bespoke::Layout* irgenLayout() const;

  Sort sort;
};

// Jumpstart support for our layout selection decisions.
void serializeBespokeLayouts(ProfDataSerializer& ser);
void deserializeBespokeLayouts(ProfDataDeserializer& des);

}}
