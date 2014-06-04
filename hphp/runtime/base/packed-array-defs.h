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
#ifndef incl_HPHP_PACKED_ARRAY_DEFS_H_
#define incl_HPHP_PACKED_ARRAY_DEFS_H_

namespace HPHP {

//////////////////////////////////////////////////////////////////////

constexpr uint32_t kPackedSmallSize = 3; // same as mixed-array for now

// Max size allowed by PackedArray's capacity encoding scheme
constexpr uint32_t kMaxPackedCap = 0xFF00FF00ul;

constexpr uint32_t kPackedCapCodeThreshold = 0x10000ul;

//////////////////////////////////////////////////////////////////////

/*
 * Return the payload from a ArrayData* that is kPackedKind.
 */
ALWAYS_INLINE
TypedValue* packedData(const ArrayData* arr) {
  return const_cast<TypedValue*>(
    reinterpret_cast<const TypedValue*>(arr + 1)
  );
}

ALWAYS_INLINE
ArrayData* getArrayFromPackedData(const TypedValue* tv) {
  return const_cast<ArrayData*>(
    reinterpret_cast<const ArrayData*>(tv) - 1
  );
}

/**
 * Packed arrays use an encoding scheme for their capacity field so that
 * capacities up to almost 2^32 can be supported using only 3 bytes:
 *   cap = capCode < 0x10000 ? capCode : (capCode - 0xFF00) * 0x100
 *
 * The encoding breaks 3-byte capacity codes (capCodes) into two ranges.
 * Codes 0 - 65535 are mapped to capacities 0 - 65535 in increments of 1.
 * Codes 65536 - 16777215 are mapped to capacities 65536 - 4278255360 in
 * increments of 256. This scheme works out well in a couple ways:
 *   - No meaningful loss of granularity for the capacity field.
 *   - cap == capCode in the common case, which comes in handy.
 *   - No multiplication or division needed.
 */

ALWAYS_INLINE
uint32_t packedCapToCode(uint32_t cap) {
  assert(cap <= kMaxPackedCap);
  if (UNLIKELY(cap > kPackedCapCodeThreshold)) {
    auto const capCode = (cap + 0xFF00FFul) >> 8;
    assert(capCode <= 0xFFFFFFul && capCode <= cap);
    return capCode;
  }
  return cap;
}

ALWAYS_INLINE
uint32_t packedCodeToCap(uint32_t capCode) {
  assert(capCode <= 0xFFFFFFul);
  if (UNLIKELY(capCode > kPackedCapCodeThreshold)) {
    auto const cap = (capCode - 0xFF00ul) << 8;
    assert(cap <= kMaxPackedCap && capCode <= cap);
    return cap;
  }
  return capCode;
}

ALWAYS_INLINE
uint32_t roundUpPackedCap(uint32_t cap) {
  assert(cap <= kMaxPackedCap);
  if (UNLIKELY(cap > kPackedCapCodeThreshold)) {
    cap = (cap + 0xFFlu) & ~0xFFul;
  }
  // The capacity should not change if it round trips into
  // encoded form and back
  assert(packedCodeToCap(packedCapToCode(cap)) == cap);
  return cap;
}

ALWAYS_INLINE
bool sizeLessThanPackedCapCode(uint32_t size, uint32_t packedCapCode) {
  assert(packedCapCode <= 0xFFFFFFul);
  // Try comparing against m_packedCapCode first so that we can
  // avoid computing the capacity in the common case
  if (LIKELY(size < packedCapCode)) {
    assert(size < packedCodeToCap(packedCapCode));
    return true;
  }
  if (LIKELY(packedCapCode <= kPackedCapCodeThreshold)) {
    assert(!(size < packedCodeToCap(packedCapCode)));
    return false;
  }
  auto const cap = (packedCapCode - 0xFF00ul) << 8;
  assert(cap <= kMaxPackedCap && packedCapCode <= cap);
  assert((size < packedCodeToCap(packedCapCode)) == (size < cap));
  return size < cap;
}

//////////////////////////////////////////////////////////////////////

}

#endif
