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
#include "hphp/runtime/base/string-data.h"

namespace HPHP {

// Forward declarations needed for getMask and checkInvariants.
namespace jit { struct Type; }
struct VanillaDict;

/*
 * We use four bits in the VanillaDict header to track a bit-set that tells us
 * if the VanillaDict contains certain types of keys. In particular, we track
 * int keys, static str keys, non-static str keys, and tombstones.
 *
 * Whenever we add a key to a VanillaDict, we set a bit in this bit-set. Doing
 * so means that the bit-set is conservative: if a key is in the array, then
 * the corresponding bit will be set. However, we may have bits in the bitset
 * that correspond to keys that aren't present - for example, if we add an int
 * key to the array and then remove it, we'll leave the bit set.
 *
 * Despite this conservative behavior, this bitset can be quite useful for
 * optimizations, because we don't often remove keys from arrays. For example,
 * if the bit for non-static str keys is unset, we can skip releasing keys.
 */
struct ArrayKeyTypes {
  /*
   * Equality operator (used in assertions).
   */
  bool operator==(ArrayKeyTypes other) const {
    return m_bits == other.m_bits;
  }

  /*
   * A variety of getters for the key-types bitset. Because of our conservative
   * tracking, "may have" functions may return false positive, and "must have"
   * functions may return false negatives; callers must handle this fact.
   */
  bool mayIncludeCounted() const {
    return (m_bits & kNonStaticStrKey) != 0;
  }
  bool mayIncludeTombstone() const {
    return (m_bits & kTombstoneKey) != 0;
  }
  bool mustBeInts() const {
    return (m_bits & ~kIntKey) == 0;
  }
  bool mustBeStrs() const {
    return (m_bits & ~(kStaticStrKey | kNonStaticStrKey)) == 0;
  }
  bool mustBeStaticStrs() const {
    return (m_bits & ~kStaticStrKey) == 0;
  }

  /*
   * Call this helper to get a 1-byte mask to test against when JITing code
   * to optimistically check the keys of a VanillaDict. If you and this mask
   * with the key bitset and the result is zero, then all keys match `type`.
   *
   * Some types can't be tested against this bitset; for these types, this
   * method will return std::nullopt.
   */
  static Optional<uint8_t> getMask(const jit::Type& type);

  /*
   * Call these methods to get a version of this bitset that can be stored
   * directly to a free byte in an ArrayData.
   */
  static ArrayKeyTypes Empty() {
    return Init(0);
  }
  static ArrayKeyTypes Ints() {
    return Init(kIntKey);
  }
  static ArrayKeyTypes StaticStrs() {
    return Init(kStaticStrKey);
  }
  static ArrayKeyTypes Strs() {
    return Init(static_cast<uint8_t>(kStaticStrKey | kNonStaticStrKey));
  }

  /*
   * Call these methods when performing the appropriate bulk operation.
   */
  void copyFrom(ArrayKeyTypes other, bool compact) {
    m_bits |= other.m_bits & (compact ? ~kTombstoneKey : 0xff);
  }
  void makeCompact() {
    m_bits &= ~kTombstoneKey;
  }
  void makeStatic() {
    if (m_bits & kNonStaticStrKey) {
      m_bits = (m_bits & ~kNonStaticStrKey) | kStaticStrKey;
    }
  }
  void renumberKeys() {
    m_bits = kIntKey;
  }

  /*
   * Call these methods when inserting a new key into the array.
   */
  void recordInt() {
    m_bits |= kIntKey;
  }
  void recordStr(const StringData* sd) {
    m_bits |= sd->isStatic() ? kStaticStrKey : kNonStaticStrKey;
  }
  void recordNonStaticStr() {
    m_bits |= kNonStaticStrKey;
  }
  void recordTombstone() {
    m_bits |= kTombstoneKey;
  }

  /*
   * Check that m_bits is a valid key types bitset for the given VanillaDict.
   * This check is very slow - it requires a full traversal of the array.
   */
  bool checkInvariants(const VanillaDict* ad) const;

private:
  static ArrayKeyTypes Init(uint8_t bits) {
    auto result = ArrayKeyTypes{};
    result.m_bits = bits;
    return result;
  }

  static constexpr uint8_t kNonStaticStrKey = 0b0001;
  static constexpr uint8_t kStaticStrKey    = 0b0010;
  static constexpr uint8_t kIntKey          = 0b0100;
  static constexpr uint8_t kTombstoneKey    = 0b1000;

  uint8_t m_bits;
};

} // namespace HPHP

