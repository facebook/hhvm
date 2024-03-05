/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <array>
#include <type_traits>

#include <folly/Portability.h>
#include <folly/Utility.h>
#include <folly/io/Cursor.h>
#include <folly/lang/Bits.h>
#include <folly/portability/Builtins.h>
#include <thrift/lib/cpp2/type/Id.h>

#if defined(__cpp_lib_bitops) && __cpp_lib_bitops >= 201907L
#include <bit>
#endif

// We need 64-bit for __mm_extra_epi64 and _pext_u64. MSVC support seems to be
// difficult to detect, so disable the BMI2 and SIMD versions entirely there.
#if defined(__BMI2__) && FOLLY_X64 && !defined(_MSC_VER)
#define THRIFT_UTIL_VARINTUTILS_BMI2_DECODER 1
#else
#define THRIFT_UTIL_VARINTUTILS_BMI2_DECODER 0
#endif

#if FOLLY_SSE_PREREQ(4, 1) && FOLLY_X64 && !defined(_MSC_VER)
#define THRIFT_UTIL_VARINTUTILS_SIMD_DECODER 1
#else
#define THRIFT_UTIL_VARINTUTILS_SIMD_DECODER 0
#endif

#if THRIFT_UTIL_VARINTUTILS_BMI2_DECODER || THRIFT_UTIL_VARINTUTILS_SIMD_DECODER
#include <immintrin.h>
#endif

namespace apache {
namespace thrift {

namespace util {

namespace detail {

template <typename T>
constexpr std::make_signed_t<T> zigzagToSignedInt(T n) {
  using Unsigned = std::make_unsigned_t<T>;
  return ((Unsigned)n & 1) ? ~((Unsigned)n >> 1) : ((Unsigned)n >> 1);
}

template <typename T>
constexpr std::make_unsigned_t<T> signedIntToZigzag(T n) {
  using Signed = std::make_signed_t<T>;
  using Unsigned = std::make_unsigned_t<T>;
  return ((Unsigned)n << 1) ^ (Unsigned)((Signed)n >> (sizeof(n) * 8 - 1));
}

template <class T, class CursorT>
void readVarintSlow(CursorT& c, T& value) {
  // ceil(sizeof(T) * 8) / 7
  static const size_t maxSize = (8 * sizeof(T) + 6) / 7;
  T retVal = 0;
  uint8_t shift = 0;
  uint8_t rsize = 0;
  while (true) {
    uint8_t byte = c.template read<uint8_t>();
    rsize++;
    retVal |= (uint64_t)(byte & 0x7f) << shift;
    shift += 7;
    if (!(byte & 0x80)) {
      value = retVal;
      return;
    }
    if (rsize >= maxSize) {
      // Too big for return type
      throw std::out_of_range("invalid varint read");
    }
  }
}

// This is a simple function that just throws an exception. It is defined out
// line to make the caller (readVarint) smaller and simpler (assembly-wise),
// which gives us 5% perf win (even when the exception is not actually thrown).
[[noreturn]] void throwInvalidVarint();

template <class T>
size_t readVarintMediumSlowUnrolled(T& value, const uint8_t* p) {
  uint64_t result;
  const uint8_t* start = p;
  do {
    uint64_t byte; // byte is uint64_t so that all shifts are 64-bit
    // clang-format off
      byte = *p++; result  = (byte & 0x7f);       if (!(byte & 0x80)) break;
      byte = *p++; result |= (byte & 0x7f) <<  7; if (!(byte & 0x80)) break;
      if (sizeof(T) <= 1) throwInvalidVarint();
      byte = *p++; result |= (byte & 0x7f) << 14; if (!(byte & 0x80)) break;
      if (sizeof(T) <= 2) throwInvalidVarint();
      byte = *p++; result |= (byte & 0x7f) << 21; if (!(byte & 0x80)) break;
      byte = *p++; result |= (byte & 0x7f) << 28; if (!(byte & 0x80)) break;
      if (sizeof(T) <= 4) throwInvalidVarint();
      byte = *p++; result |= (byte & 0x7f) << 35; if (!(byte & 0x80)) break;
      byte = *p++; result |= (byte & 0x7f) << 42; if (!(byte & 0x80)) break;
      byte = *p++; result |= (byte & 0x7f) << 49; if (!(byte & 0x80)) break;
      byte = *p++; result |= (byte & 0x7f) << 56; if (!(byte & 0x80)) break;
      byte = *p++; result |= (byte & 0x7f) << 63; if (!(byte & 0x80)) break;
    // clang-format on
    throwInvalidVarint();
  } while (false);
  value = static_cast<T>(result);
  return p - start;
}

// The fast-path of the optimized medium-slow paths. Decodes the first two bytes
// of the varint (into result).
// Returns true if parsing is finished after those two bytes, and false
// otherwise. Either way, the data bits of the first two bytes are stored into
// the low bits of result.
FOLLY_ALWAYS_INLINE bool tryReadFirstTwoBytesU64(
    uint64_t& result, const uint8_t* p) {
  // This is only called from mediumSlow pathways after we've done size
  // validation. In particular, we should know that it's not a
  // single-byte-encoded varint, and that there's space in the buffer for the
  // maximum size an encoded varint can be.
  DCHECK((p[0] & 0x80) != 0);
  uint64_t hi = p[1];
  result = ((hi & 0x7F) << 7) | (p[0] & 0x7f);
  return (hi & 0x80) == 0;
}

#if THRIFT_UTIL_VARINTUTILS_SIMD_DECODER
FOLLY_ALWAYS_INLINE size_t
readContiguousVarintMediumSlowU64SIMD(uint64_t& value, const uint8_t* p) {
  uint64_t result;
  if (tryReadFirstTwoBytesU64(result, p)) {
    value = result;
    return 2;
  }
  p += 2;
  // This has alternating data bits and continuation bits, then, after the
  // first 0 continuation bit, junk (well, message data that we're not
  // interested in for varint decoding purposes).
  uint64_t bits = folly::loadUnaligned<uint64_t>(p);
  // This has 1s in all bits in the encoded int except the last continuation
  // bit, then, after that, junk.
  uint64_t bitsOnlyContinations = bits | 0x7F7F7F7F7F7F7F7FULL;
  // This has all zeros in all continuation and data bits in the int except
  // for the last continuation bit, which is a 1. After that, junk.
  uint64_t lastContinuationBitSet = bitsOnlyContinations + 1;
  if (lastContinuationBitSet == 0) {
    // Last continuation bit was the last bit in the uint64_t, and it was a 1.
    throwInvalidVarint();
  }
  size_t intBytes = __builtin_ctzll(lastContinuationBitSet) / 8 + 1;
  // "Extract lowest 1 bit" idiom. Use `~lastContinuationBitSet + 1` instead
  // of `-lastContinuationBitSet` to avoid an MSVC warning.
  uint64_t solelyLastContinuationBitSet =
      lastContinuationBitSet & (~lastContinuationBitSet + 1);
  // Mask out all the junk bits.
  uint64_t dataAndContinuationBits = bits & (solelyLastContinuationBitSet - 1);
  uint64_t dataBits64 = dataAndContinuationBits & 0x7F7F7F7F7F7F7F7FULL;
  // clang-format off
    // dataBits will be:
    // 15        14        13        12        11        10        9         8
    // [00000000][00000000][00000000][00000000][00000000][00000000][00000000][00000000]
    // 7         6         5         4         3         2         1         0
    // [0AAAAAAa][0BBBBBBB][0CCCCCCC][0Ddddddd][0EEeeeee][0FFFffff][0GGGGggg][0HHHHHhh]
    __m128i dataBits = _mm_set_epi64x(0, dataBits64);
    // alternatingZeros will be:
    // 15        14        13        12        11        10        9         8
    // [00000000][0AAAAAAa][00000000][0BBBBBBB][00000000][0CCCCCCC][00000000][0Ddddddd]
    // 7         6         5         4         3         2         1         0
    // [00000000][0EEeeeee][00000000][0FFFffff][00000000][0GGGGggg][00000000][0HHHHHhh]
    __m128i alternatingZeros = _mm_cvtepu8_epi16(dataBits);
    // shifted will be:
    // 15        14        13        12        11        10        9         8
    // [00AAAAAA][a0000000][00000000][0BBBBBBB][00000000][CCCCCCC0][0000000D][dddddd00]
    // 7         6         5         4         3         2         1         0
    // [000000EE][eeeee000][00000FFF][ffff0000][0000GGGG][ggg00000][000HHHHH][hh000000]
    // (We implement the shift as a multiply just because of ISA limitations).
    __m128i shifted = _mm_mullo_epi16(alternatingZeros, _mm_set_epi16(
          1 << 7, 1 << 0, 1 << 1, 1 << 2,
          1 << 3, 1 << 4, 1 << 5, 1 << 6));
    // shuffled will be:
    // 15        14        13        12        11        10        9         8
    // [0BBBBBBB][CCCCCCC0][dddddd00][eeeee000][ffff0000][ggg00000][hh000000][00000000]
    // 7         6         5         4         3         2         1         0
    // [a0000000][0000000D][000000EE][00000FFF][0000GGGG][000HHHHH][00000000][00000000]
    __m128i shuffled = _mm_shuffle_epi8(shifted, _mm_set_epi8(
          12, 10, 8, 6, 4, 2, 0, -1,
          14, 9, 7, 5, 3, 1, -1, -1));
  // clang-format on
  uint64_t highData1 = _mm_extract_epi64(shuffled, 0);
  uint64_t highData2 = _mm_extract_epi64(shuffled, 1);
  result = result + highData1 + highData2;
  value = result;
  return 2 + intBytes;
}
#endif // THRIFT_UTIL_VARINTUTILS_SIMD_DECODER

#if THRIFT_UTIL_VARINTUTILS_BMI2_DECODER
inline size_t readContiguousVarintMediumSlowU64BMI2(
    uint64_t& value, const uint8_t* p) {
  uint64_t result;
  if (tryReadFirstTwoBytesU64(result, p)) {
    value = result;
    return 2;
  }
  p += 2;
  // This has alternating data bits and continuation bits, then, after the
  // first 0 continuation bit, junk (well, message data that we're not
  // interested in for varint decoding purposes).
  uint64_t bits = folly::loadUnaligned<uint64_t>(p);

  const uint64_t kDataBitMask = 0x7F7F7F7F7F7F7F7FULL;
  const uint64_t allDataBitsSet = bits | kDataBitMask;
  const uint64_t oneInLowestUnsetContinuationBitWithTrailingBitsZero =
      allDataBitsSet + 1;
  // NOTE: by hoisting the increment of allDataBitsSet above this
  // check, we can perform the check against zero rather than -1.
  // This accelerates the check because the INC instruction will set
  // ZF if the result is 0, netting a savings of one CMP instruction.
  if (oneInLowestUnsetContinuationBitWithTrailingBitsZero == 0) {
    // The maximum bytes of int64 would be ceil(64/7)=10, as we had used
    // tryReadFirstTwoBytesU64 to read 2 bytes, there would be 8 bytes leftover
    // in maximum.
    //
    // Overall, we don't expect the continuation bit mask would be all set.
    throwInvalidVarint();
  }
  // By reset data bits and toggle the continuation bits, the tailing zeros
  // should be intBytes*8-1
#if defined(__cpp_lib_bitops) && __cpp_lib_bitsops >= 201907L
  // __builtin_ctzll(0) is defined as UB, even though TZCNT does what we want in
  // that case. std::countr_zero is the same thing without the UB.
  size_t maskShift =
      std::countr_zero(oneInLowestUnsetContinuationBitWithTrailingBitsZero);
#else
  // We settle for folly::findFirstSet and fix up the result if we have to.
  size_t maskShift =
      folly::findFirstSet(oneInLowestUnsetContinuationBitWithTrailingBitsZero);
  if (maskShift == 0) {
    maskShift = 64;
  } else {
    maskShift--;
  }
#endif
  size_t intBytes = (maskShift >> 3) + 1;

  uint64_t mask = (1ULL << maskShift) - 1;
  // You might think it would make more sense to to the pext first and mask
  // afterwards (avoiding having two pexts in a single dependency chain at 3
  // cycles / pop); this seems not to be borne out in microbenchmarks. The
  // mask you need ends up being more complicated to compute.
  uint64_t highBits = _pext_u64((bits & mask), kDataBitMask);
  result |= (highBits << 14);

  value = result;
  return 2 + intBytes;
}
#endif // THRIFT_UTIL_VARINTUTILS_BMI2_DECODER

template <class T, class CursorT>
void readVarintMediumSlow(CursorT& c, T& value, const uint8_t* p, size_t len) {
  static_assert(
      sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
      "Trying to deserialize into an unsupported type");

  static const size_t maxSize = (8 * sizeof(T) + 6) / 7;

  if (FOLLY_LIKELY(len >= maxSize)) {
    size_t bytesRead;
    if (sizeof(T) <= 4) {
      bytesRead = readVarintMediumSlowUnrolled(value, p);
    } else {
      uint64_t result;
#if THRIFT_UTIL_VARINTUTILS_BMI2_DECODER
      bytesRead = readContiguousVarintMediumSlowU64BMI2(result, p);
#elif THRIFT_UTIL_VARINTUTILS_SIMD_DECODER
      bytesRead = readContiguousVarintMediumSlowU64SIMD(result, p);
#else
      bytesRead = readVarintMediumSlowUnrolled(result, p);
#endif
      value = static_cast<T>(result);
    }
    c.skipNoAdvance(bytesRead);
  } else {
    readVarintSlow(c, value);
  }
}
} // namespace detail

template <class T, class CursorT>
void readVarint(CursorT& c, T& value) {
  const uint8_t* p = c.data();
  size_t len = c.length();
  if (len > 0 && !(*p & 0x80)) {
    value = static_cast<T>(*p);
    c.skipNoAdvance(1);
  } else {
    detail::readVarintMediumSlow(c, value, p, len);
  }
}

template <class T, class CursorT>
T readVarint(CursorT& c) {
  T value;
  readVarint(c, value);
  return value;
}

namespace detail {

// Cursor class must have ensure() and append() (e.g. QueueAppender)
template <class Cursor, class T>
uint8_t writeVarintSlow(Cursor& c, T value) {
  enum { maxSize = (8 * sizeof(T) + 6) / 7 };
  auto unval = folly::to_unsigned(value);

  c.ensure(maxSize);

  uint8_t* p = c.writableData();
  uint8_t* orig_p = p;
  // precondition: (value & ~0x7f) != 0
  do {
    // clang-format off
    *p++ = ((unval & 0x7f) | 0x80); unval = unval >> 7; if ((unval & ~0x7f) == 0) break;
    *p++ = ((unval & 0x7f) | 0x80); unval = unval >> 7; if ((unval & ~0x7f) == 0) break;
    *p++ = ((unval & 0x7f) | 0x80); unval = unval >> 7; if ((unval & ~0x7f) == 0) break;
    *p++ = ((unval & 0x7f) | 0x80); unval = unval >> 7; if ((unval & ~0x7f) == 0) break;
    *p++ = ((unval & 0x7f) | 0x80); unval = unval >> 7; if ((unval & ~0x7f) == 0) break;
    *p++ = ((unval & 0x7f) | 0x80); unval = unval >> 7; if ((unval & ~0x7f) == 0) break;
    *p++ = ((unval & 0x7f) | 0x80); unval = unval >> 7; if ((unval & ~0x7f) == 0) break;
    *p++ = ((unval & 0x7f) | 0x80); unval = unval >> 7; if ((unval & ~0x7f) == 0) break;
    *p++ = ((unval & 0x7f) | 0x80); unval = unval >> 7;
    // clang-format on
  } while (false);

  *p++ = static_cast<uint8_t>(unval);
  c.append(p - orig_p);
  return static_cast<uint8_t>(p - orig_p);
}

} // namespace detail

template <class Cursor, class T>
uint8_t writeVarintUnrolled(Cursor& c, T value) {
  if (FOLLY_LIKELY((value & ~0x7f) == 0)) {
    c.template write<uint8_t>(static_cast<uint8_t>(value));
    return 1;
  }

  return detail::writeVarintSlow(c, value);
}

#ifdef __BMI2__

template <class Cursor, class T>
uint8_t writeVarintBMI2(Cursor& c, T valueS) {
  auto value = folly::to_unsigned(valueS);
  if (FOLLY_LIKELY((value & ~0x7f) == 0)) {
    c.template write<uint8_t>(static_cast<uint8_t>(value));
    return 1;
  }

  if /* constexpr */ (sizeof(T) == 1) {
    c.template write<uint16_t>(static_cast<uint16_t>(value | 0x100));
    return 2;
  }

  constexpr uint64_t kMask = 0x8080808080808080ULL;
  static const std::array<uint8_t, 64> kShift = []() {
    std::array<uint8_t, 64> v = {};
    for (size_t i = 0; i < 64; i++) {
      uint8_t byteShift = folly::to_narrow(i == 0 ? 0 : (8 - ((63 - i) / 7)));
      v[i] = byteShift * 8;
    }
    return v;
  }();
  static const std::array<uint8_t, 64> kSize = []() {
    std::array<uint8_t, 64> v = {};
    for (size_t i = 0; i < 64; i++) {
      v[i] = folly::to_narrow(((63 - i) / 7) + 1);
    }
    return v;
  }();

  auto clzll = __builtin_clzll(static_cast<uint64_t>(value));
  // Only the first 56 bits of @value will be deposited in @v.
  uint64_t v = _pdep_u64(value, ~kMask) | (kMask >> kShift[clzll]);
  uint8_t size = kSize[clzll];

  if /* constexpr */ (sizeof(T) < sizeof(uint64_t)) {
    c.template write<uint64_t>(v, size);
  } else {
    // Ensure max encoding space for u64 varint (10 bytes).
    // Write 56 bits using pdep and the other 8 bits manually.
    // Write 10B to @c, but only update the size using @size.
    // (Writing exta bytes is faster than branching based on @size.)
    c.ensure(10);
    uint8_t* p = c.writableData();
    folly::storeUnaligned<uint64_t>(p, v);
    p[sizeof(uint64_t) + 0] = static_cast<uint64_t>(value) >> 56;
    p[sizeof(uint64_t) + 1] = 1;
    c.append(size);
  }
  return size;
}

template <class Cursor, class T>
uint8_t writeVarint(Cursor& c, T value) {
  return writeVarintBMI2(c, value);
}

#else // __BMI2__

template <class Cursor, class T>
uint8_t writeVarint(Cursor& c, T value) {
  return writeVarintUnrolled(c, value);
}

#endif // __BMI2__

inline int32_t zigzagToI32(uint32_t n) {
  return detail::zigzagToSignedInt(n);
}

inline int64_t zigzagToI64(uint64_t n) {
  return detail::zigzagToSignedInt(n);
}

constexpr inline uint32_t i32ToZigzag(const int32_t n) {
  return detail::signedIntToZigzag(n);
}

constexpr inline uint64_t i64ToZigzag(const int64_t n) {
  return detail::signedIntToZigzag(n);
}

inline uint32_t toI32ZigZagOrdinal(size_t pos) {
  return apache::thrift::util::i32ToZigzag(
      static_cast<int32_t>(type::toOrdinal(pos)));
}

inline size_t fromI32ZigZagOrdinal(uint32_t pos) {
  return type::toPosition(
      type::Ordinal(apache::thrift::util::zigzagToI32(pos)));
}

template <class Cursor, class T>
FOLLY_ALWAYS_INLINE void readZigzaggedVarint(Cursor& c, T& out) {
  static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);
  folly::compiler_may_unsafely_assume_separate_storage(&c, c.data());
  // By using the 4-byte version of readVarint, we can cut down on template
  // instantiations and reduce icache pressure.
  if constexpr (sizeof(T) == 2) {
    int32_t value;
    readVarint(c, value);
    out = (int16_t)detail::zigzagToSignedInt(value);
  } else {
    T value;
    readVarint(c, value);
    out = detail::zigzagToSignedInt(value);
  }
}

} // namespace util
} // namespace thrift
} // namespace apache
