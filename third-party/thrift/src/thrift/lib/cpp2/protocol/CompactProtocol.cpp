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

#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

#include <folly/Conv.h>

#if FOLLY_ARM_FEATURE_NEON_SVE_BRIDGE

#include <arm_neon.h>
#include <arm_sve.h>

#include <arm_neon_sve_bridge.h> // @manual

#endif

namespace apache::thrift {

[[noreturn]] void CompactProtocolReader::throwBadProtocolIdentifier() {
  throw TProtocolException(
      TProtocolException::BAD_VERSION, "Bad protocol identifier");
}

[[noreturn]] void CompactProtocolReader::throwBadProtocolVersion() {
  throw TProtocolException(
      TProtocolException::BAD_VERSION, "Bad protocol version");
}

[[noreturn]] void CompactProtocolReader::throwBadType(const uint8_t type) {
  throw TProtocolException(
      folly::to<std::string>("don't know what type: ", type));
}

void CompactProtocolReader::readFieldBeginWithStateMediumSlow(
    StructReadState& state, int16_t prevFieldId) {
  auto byte = *in_.data();
  in_.skipNoAdvance(1);

  readFieldBeginWithStateImpl(state, prevFieldId, byte);
}

#if FOLLY_ARM_FEATURE_NEON_SVE_BRIDGE

// Compute signedIntToZigzag on Eight 16-bit vars
static uint16x8_t wideSignedInt16ToZigzag(uint16x8_t n) {
  uint16x8_t cmpV = vcltzq_s16(vreinterpretq_s16_u16(n));
  n = vshlq_n_u16(n, 1);
  n = veorq_u16(n, cmpV);
  return n;
}

// Compute signedIntToZigzag on four 32-bit vars
static uint32x4_t wideSignedInt32ToZigzag(uint32x4_t n) {
  uint32x4_t cmpV = vcltzq_s32(vreinterpretq_s32_u32(n));
  n = vshlq_n_u32(n, 1);
  n = veorq_u32(n, cmpV);
  return n;
}

// Compute signedIntToZigzag on two 64-bit vars
static uint64x2_t wideSignedInt64ToZigzag(uint64x2_t n) {
  uint64x2_t cmpV = vcltzq_s64(vreinterpretq_s64_u64(n));
  n = vshlq_n_u64(n, 1);
  n = veorq_u64(n, cmpV);
  return n;
}

// NEON variations of function signedIntToZigzag, from VarintUtils-inl.h
template <typename T>
static poly128_t wideSignedIntToZigzag(poly128_t n) {
  if constexpr (sizeof(T) == 2) {
    return vreinterpretq_p128_u16(
        wideSignedInt16ToZigzag(vreinterpretq_u16_p128(n)));
  } else if constexpr (sizeof(T) == 4) {
    return vreinterpretq_p128_u32(
        wideSignedInt32ToZigzag(vreinterpretq_u32_p128(n)));
  } else {
    return vreinterpretq_p128_u64(
        wideSignedInt64ToZigzag(vreinterpretq_u64_p128(n)));
  }
}

template <class Cursor, class T>
static uint64_t writeTwoVarintSve(Cursor& c, uint64x2_t vec) {
  c.ensure(sizeof(T) == 4 ? 13 : 20);

  uint64_t upcastedValA = vec[0];
  uint64_t upcastedValB = vec[1];
  svuint8_t mask = svdup_n_u8(0x7f);
  svuint64_t clzMask = svreinterpret_u64_u8(svdup_n_u8(0x80));
  svbool_t clzMaskCheck;
  uint64x2_t zeroCheck = vceqzq_u64(vec);

  if constexpr (sizeof(T) == 8) {
    clzMaskCheck = svcmple_n_u64(
        svptrue_b64(), svset_neonq_u64(svundef_u64(), vec), (1ull << 56) - 1);
  } else {
    clzMaskCheck = svptrue_b64();
  }

  vec = svget_neonq_u64(svbdep_u64(
      svset_neonq_u64(svundef_u64(), vec), svreinterpret_u64_u8(mask)));

  zeroCheck = svget_neonq_u64(svand_n_u64_x(
      svptrue_b64(), svset_neonq_u64(svundef_u64(), zeroCheck), 1));

  svuint64_t clzV =
      svclz_u64_x(svptrue_b64(), svset_neonq_u64(svundef_u64(), vec));

  svuint64_t sizeSV = svlsr_n_u64_x(svptrue_b64(), clzV, 3);

  clzV = svand_n_u64_x(svptrue_b64(), clzV, 120);

  sizeSV = svsubr_n_u64_x(svptrue_b64(), sizeSV, 8);

  sizeSV = svset_neonq_u64(
      svundef_u64(), vaddq_u64(svget_neonq_u64(sizeSV), zeroCheck));

  clzV = svadd_n_u64_x(svptrue_b64(), clzV, 8);

  clzMask = svlsr_u64_m(clzMaskCheck, clzMask, clzV);

  uint64_t sizeA = sizeSV[0];
  uint64_t sizeB = sizeSV[1];

  vec = vorrq_u64(vec, svget_neonq_u64(clzMask));

  uint8_t* p = c.writableData();

  if constexpr (sizeof(T) == sizeof(uint32_t)) {
    vst1q_lane_u64(p, vec, 0);
    vst1q_lane_u64(p + sizeA, vec, 1);
  } else {
    vst1q_lane_u64(p, vec, 0);
    if (upcastedValA >= (1ull << 56)) {
      p[8] = upcastedValA >> 56;
      upcastedValA >>= 63;
      p[9] = upcastedValA;
      sizeA = 9 + upcastedValA;
    }
    p += sizeA;
    vst1q_lane_u64(p, vec, 1);
    if (upcastedValB >= (1ull << 56)) {
      p[8] = upcastedValB >> 56;
      upcastedValB >>= 63;
      p[9] = upcastedValB;
      sizeB = 9 + upcastedValB;
    }
  }

  uint64_t size = sizeA + sizeB;

  c.append(size);
  return size;
}

template <class Cursor, class T>
static uint64_t writeTwoVarint64Sve(Cursor& c, uint64x2_t vec) {
  return writeTwoVarintSve<Cursor, T>(c, vec);
}

template <class Cursor, class T>
static uint64_t writeTwoVarint32Sve(Cursor& c, uint64x2_t vec) {
  return writeTwoVarintSve<Cursor, T>(c, vec);
}

template <class Cursor, class T>
static uint64_t writeFourVarint32Sve(
    Cursor& c, uint64x2_t vec1, uint64x2_t vec2) {
  svuint8_t bdepMask = svdup_n_u8(0x7f);
  uint64x2_t clzMask = vreinterpretq_u64_u8(vdupq_n_u8(0xff));

  uint64x2_t zeroCheck1 = vceqzq_u64(vec1);
  uint64x2_t zeroCheck2 = vceqzq_u64(vec2);

  c.ensure(23);

  vec1 = svget_neonq_u64(svbdep_u64(
      svset_neonq_u64(svundef_u64(), vec1), svreinterpret_u64_u8(bdepMask)));
  vec2 = svget_neonq_u64(svbdep_u64(
      svset_neonq_u64(svundef_u64(), vec2), svreinterpret_u64_u8(bdepMask)));

  zeroCheck1 = svget_neonq_u64(svand_n_u64_x(
      svptrue_b64(), svset_neonq_u64(svundef_u64(), zeroCheck1), 1));

  zeroCheck2 = svget_neonq_u64(svand_n_u64_x(
      svptrue_b64(), svset_neonq_u64(svundef_u64(), zeroCheck2), 1));

  svuint64_t clzV1 =
      svclz_u64_x(svptrue_b64(), svset_neonq_u64(svundef_u64(), vec1));

  svuint64_t clzV2 =
      svclz_u64_x(svptrue_b64(), svset_neonq_u64(svundef_u64(), vec2));

  vec1 = svget_neonq_u8(svorr_n_u8_x(
      svptrue_b8(),
      svset_neonq_u8(svundef_u8(), vreinterpretq_u8_u64(vec1)),
      0x80));

  vec2 = svget_neonq_u8(svorr_n_u8_x(
      svptrue_b8(),
      svset_neonq_u8(svundef_u8(), vreinterpretq_u8_u64(vec2)),
      0x80));

  svuint64_t sizeSV1 = svlsr_n_u64_x(svptrue_b64(), clzV1, 3);

  svuint64_t sizeSV2 = svlsr_n_u64_x(svptrue_b64(), clzV2, 3);

  svuint64_t clzMask1 = svlsr_u64_x(
      svptrue_b64(), svset_neonq_u64(svundef_u64(), clzMask), clzV1);

  svuint64_t clzMask2 = svlsr_u64_x(
      svptrue_b64(), svset_neonq_u64(svundef_u64(), clzMask), clzV2);

  sizeSV1 = svset_neonq_u64(
      svundef_u64(), vsubq_u64(svget_neonq_u64(sizeSV1), zeroCheck1));

  sizeSV2 = svset_neonq_u64(
      svundef_u64(), vsubq_u64(svget_neonq_u64(sizeSV2), zeroCheck2));

  uint64_t sizeA = 8 - sizeSV1[0];
  uint64_t sizeB = 8 - sizeSV1[1];
  uint64_t sizeC = 8 - sizeSV2[0];
  uint64_t sizeD = 8 - sizeSV2[1];

  vec1 = vandq_u64(vec1, svget_neonq_u64(clzMask1));

  vec2 = vandq_u64(vec2, svget_neonq_u64(clzMask2));

  uint8_t* p = c.writableData();

  vst1q_lane_u64(p, vec1, 0);
  p += sizeA;
  vst1q_lane_u64(p, vec1, 1);
  p += sizeB;
  vst1q_lane_u64(p, vec2, 0);
  p += sizeC;
  vst1q_lane_u64(p, vec2, 1);

  uint64_t size = sizeA + sizeB + sizeC + sizeD;

  c.append(size);
  return size;
}

template <class Cursor, class T>
static uint64_t writeFourVarint16Sve(Cursor& c, uint32x4_t vec) {
  svuint8_t mask = svdup_n_u8(0x7f);
  uint64x2_t clzMask = vreinterpretq_u64_u8(vdupq_n_u8(0xff));
  uint32x4_t zeroCheck = vceqzq_u32(vec);

  c.ensure(13);

  vec = svget_neonq_u32(svbdep_u32(
      svset_neonq_u32(svundef_u32(), vec), svreinterpret_u32_u8(mask)));

  zeroCheck = svget_neonq_u32(svand_n_u32_x(
      svptrue_b32(), svset_neonq_u32(svundef_u32(), zeroCheck), 1));

  svuint32_t clzV = svset_neonq_u32(svundef_u32(), vclzq_u32(vec));

  svuint32_t sizeSV = svlsr_n_u32_x(svptrue_b32(), clzV, 3);

  vec = svget_neonq_u8(svorr_n_u8_x(
      svptrue_b8(),
      svset_neonq_u8(svundef_u8(), vreinterpretq_u8_u32(vec)),
      0x80));

  sizeSV = svset_neonq_u32(
      svundef_u32(), vsubq_u32(svget_neonq_u32(sizeSV), zeroCheck));

  clzMask = svget_neonq_u32(svlsr_u32_x(
      svptrue_b32(), svset_neonq_u32(svundef_u32(), clzMask), clzV));

  uint64_t sizeA = 4 - svget_neonq_u32(sizeSV)[0];
  uint64_t sizeB = 4 - sizeSV[1];
  uint64_t sizeC = 4 - sizeSV[2];
  uint64_t sizeD = 4 - sizeSV[3];

  uint64_t totalSize = sizeA + sizeB + sizeC + sizeD;

  vec = vandq_u64(vec, clzMask);

  uint8_t* p = c.writableData();

  vst1q_lane_u32(p, vec, 0);
  p += sizeA;
  vst1q_lane_u32(p, vec, 1);
  p += sizeB;
  vst1q_lane_u32(p, vec, 2);
  p += sizeC;
  vst1q_lane_u32(p, vec, 3);

  c.append(totalSize);
  return totalSize;
}

// Function used with data types that are just sent as BE/LE bytes
template <class Cursor, typename T, bool BE>
static inline size_t writeUnencodedArithmeticVector(
    Cursor& c, const T* inputPtr, size_t numElements) {
  size_t len = numElements * sizeof(T);
  size_t i = 0;
  while (i < numElements) {
    c.ensureWithinMaxGrowth(len);
    uint8_t* outPtr = c.writableData();
    size_t loopBound = std::min(numElements, (c.length() / sizeof(T)) + i);
    size_t j = 0;
    for (; i < loopBound; ++i, ++j) {
      T value = BE ? folly::Endian::big<T>(inputPtr[i]) : inputPtr[i];
      folly::storeUnaligned<T>(outPtr + j * sizeof(T), value);
    }
    c.append(j * sizeof(T));
  }
  return len;
}

template <typename T>
constexpr size_t kSimdWriteWidth = sizeof(T) == 2 ? 4 : 2;

template <typename T>
static poly128_t readArithmeticElements(const T* inputPtr) {
  // Using svptrue_b8() instead of svptrue_b32()/svptrue_b64() results in one
  // less instruction being emitted. Issue tracked in T221893168
  if constexpr (sizeof(T) == 2) {
    return vreinterpretq_p128_u32(svget_neonq_u32(svld1uh_u32(
        svptrue_b8(), reinterpret_cast<const uint16_t*>(inputPtr))));
  } else if constexpr (sizeof(T) == 4) {
    return vreinterpretq_p128_u64(svget_neonq_u64(svld1uw_u64(
        svptrue_b8(), reinterpret_cast<const uint32_t*>(inputPtr))));
  } else {
    return vreinterpretq_p128_u64(
        vld1q_u64(reinterpret_cast<const uint64_t*>(inputPtr)));
  }
}

template <class Cursor, class T>
static uint64_t writeArithmeticElements(Cursor& c, poly128_t vec) {
  if constexpr (sizeof(T) == 2) {
    return writeFourVarint16Sve<Cursor, T>(c, vreinterpretq_u32_p128(vec));
  } else {
    return writeTwoVarint64Sve<Cursor, T>(c, vreinterpretq_u64_p128(vec));
  }
}

// Function used with data types that are encoded as zigzag and compacted
template <class Cursor, typename T>
static inline size_t writeEncodedArithmeticVector(
    Cursor& c, const T* inputPtr, size_t numElements) {
  uint64_t bytesWritten = 0;
  size_t numElementsMainLoop = numElements - numElements % kSimdWriteWidth<T>;
  size_t i = 0;
  for (; i < numElementsMainLoop; i += kSimdWriteWidth<T>) {
    auto vec = readArithmeticElements<T>(inputPtr + i);
    vec = wideSignedIntToZigzag<T>(vec);
    bytesWritten += writeArithmeticElements<Cursor, T>(c, vec);
  }

  if constexpr (kSimdWriteWidth<T> == 2) {
    // only one element may be left
    if (i < numElements) {
      if constexpr (sizeof(T) == 8) {
        bytesWritten += apache::thrift::util::writeVarint(
            c, apache::thrift::util::i64ToZigzag(inputPtr[i]));
      } else {
        bytesWritten += apache::thrift::util::writeVarint(
            c, apache::thrift::util::i32ToZigzag(inputPtr[i]));
      }
    }
  } else {
    for (; i < numElements; i += 1) {
      int16_t signedVal = (int16_t)inputPtr[i];
      bytesWritten += apache::thrift::util::writeVarint(
          c, apache::thrift::util::i32ToZigzag(signedVal));
    }
  }

  return bytesWritten;
}

// Loop unrolled variant of writeEncodedArithmeticVector
// Function used with 32-bit data types, these are zigzag encoded and compacted
template <class Cursor, typename T>
static inline size_t writeEncodedArithmeticVectorInterleaved(
    Cursor& c, const T* inputPtr, size_t numElements) {
  constexpr size_t kInterleavedSimdWriteWidth = 2 * kSimdWriteWidth<T>;
  uint64_t bytesWritten = 0;
  size_t numElementsModWidth = numElements % kInterleavedSimdWriteWidth;
  size_t numElementsMainLoop = numElements - numElementsModWidth;
  size_t i = 0;
  for (; i < numElementsMainLoop; i += kInterleavedSimdWriteWidth) {
    auto vec1 = readArithmeticElements<T>(inputPtr + i);
    auto vec2 = readArithmeticElements<T>(inputPtr + i + kSimdWriteWidth<T>);
    vec1 = wideSignedIntToZigzag<T>(vec1);
    vec2 = wideSignedIntToZigzag<T>(vec2);
    bytesWritten += writeFourVarint32Sve<Cursor, T>(
        c, vreinterpretq_u64_p128(vec1), vreinterpretq_u64_p128(vec2));
  }

  if (numElementsModWidth >= kSimdWriteWidth<T>) {
    auto vec = readArithmeticElements<T>(inputPtr + i);
    vec = wideSignedIntToZigzag<T>(vec);
    bytesWritten +=
        writeTwoVarint32Sve<Cursor, T>(c, vreinterpretq_u64_p128(vec));
    i += kSimdWriteWidth<T>;
  }

  if (numElementsModWidth & 1) {
    bytesWritten += apache::thrift::util::writeVarint(
        c, apache::thrift::util::i32ToZigzag(inputPtr[i]));
  }

  return bytesWritten;
}

template <>
size_t CompactProtocolWriter::writeArithmeticVector<int64_t>(
    const int64_t* inputPtr, size_t numElements) {
  return writeEncodedArithmeticVector<QueueAppender, int64_t>(
      out_, inputPtr, numElements);
}
template <>
size_t CompactProtocolWriter::writeArithmeticVector<uint64_t>(
    const uint64_t* inputPtr, size_t numElements) {
  return writeEncodedArithmeticVector<QueueAppender, uint64_t>(
      out_, inputPtr, numElements);
}
template <>
size_t CompactProtocolWriter::writeArithmeticVector<int32_t>(
    const int32_t* inputPtr, size_t numElements) {
  return writeEncodedArithmeticVectorInterleaved<QueueAppender, int32_t>(
      out_, inputPtr, numElements);
}
template <>
size_t CompactProtocolWriter::writeArithmeticVector<uint32_t>(
    const uint32_t* inputPtr, size_t numElements) {
  return writeEncodedArithmeticVectorInterleaved<QueueAppender, uint32_t>(
      out_, inputPtr, numElements);
}
template <>
size_t CompactProtocolWriter::writeArithmeticVector<int16_t>(
    const int16_t* inputPtr, size_t numElements) {
  return writeEncodedArithmeticVector<QueueAppender, int16_t>(
      out_, inputPtr, numElements);
}
template <>
size_t CompactProtocolWriter::writeArithmeticVector<uint16_t>(
    const uint16_t* inputPtr, size_t numElements) {
  return writeEncodedArithmeticVector<QueueAppender, uint16_t>(
      out_, inputPtr, numElements);
}
template <>
size_t CompactProtocolWriter::writeArithmeticVector<int8_t>(
    const int8_t* inputPtr, size_t numElements) {
  return writeUnencodedArithmeticVector<QueueAppender, int8_t, false>(
      out_, inputPtr, numElements);
}
template <>
size_t CompactProtocolWriter::writeArithmeticVector<uint8_t>(
    const uint8_t* inputPtr, size_t numElements) {
  return writeUnencodedArithmeticVector<QueueAppender, uint8_t, false>(
      out_, inputPtr, numElements);
}
template <>
size_t CompactProtocolWriter::writeArithmeticVector<float>(
    const float* inputPtr, size_t numElements) {
  return writeUnencodedArithmeticVector<QueueAppender, float, true>(
      out_, inputPtr, numElements);
}
template <>
size_t CompactProtocolWriter::writeArithmeticVector<double>(
    const double* inputPtr, size_t numElements) {
  return writeUnencodedArithmeticVector<QueueAppender, double, true>(
      out_, inputPtr, numElements);
}

#endif // FOLLY_ARM_FEATURE_NEON_SVE_BRIDGE

// Function used with data types that are decoded from compacted zigzag
template <class Cursor, typename T>
static inline void readEncodedArithmeticVector(
    Cursor& c, T* outputPtr, size_t numElements) {
  size_t i = 0;
  size_t loopBound = numElements - 1;
  while (i < numElements) {
    folly::ByteRange currentRange = c.peekBytes();
    const uint8_t* inPtr = currentRange.data();
    constexpr size_t kMaxVarintBytes = sizeof(T) == 2
        ? util::detail::kVarintMaxBytes<int32_t>
        : util::detail::kVarintMaxBytes<T>;
    constexpr size_t kMaxBytesIter = kMaxVarintBytes * 2;
    const uint8_t* endVec = inPtr + currentRange.size() - kMaxBytesIter;
    for (; i < loopBound && inPtr <= endVec; i += 2) {
      if constexpr (sizeof(T) == 2) {
        int32_t valueA;
        int32_t valueB;
        inPtr += util::detail::readVarintMediumSlowUnrolled(valueA, inPtr);
        inPtr += util::detail::readVarintMediumSlowUnrolled(valueB, inPtr);
        valueA = util::detail::zigzagToSignedInt(valueA);
        valueB = util::detail::zigzagToSignedInt(valueB);
        outputPtr[i] = (int16_t)valueA;
        outputPtr[i + 1] = (int16_t)valueB;
      } else {
        T valueA;
        T valueB;
        inPtr += util::detail::readVarintMediumSlowUnrolled(valueA, inPtr);
        inPtr += util::detail::readVarintMediumSlowUnrolled(valueB, inPtr);
        valueA = util::detail::zigzagToSignedInt(valueA);
        valueB = util::detail::zigzagToSignedInt(valueB);
        outputPtr[i] = valueA;
        outputPtr[i + 1] = valueB;
      }
    }
    size_t consumed = inPtr - currentRange.data();
    c.skipNoAdvance(consumed);
    size_t len = currentRange.size() - consumed;
    size_t trailingLoopBound = std::min(numElements, i + len);
    while (i < trailingLoopBound) {
      if constexpr (sizeof(T) == 2) {
        // Need to finish consuming current input buffer
        int32_t value;
        util::detail::readVarintSlow(c, value);
        outputPtr[i] = (int16_t)util::detail::zigzagToSignedInt(value);
      } else {
        // Need to finish consuming current input buffer
        T value;
        util::detail::readVarintSlow(c, value);
        outputPtr[i] = util::detail::zigzagToSignedInt(value);
      }
      ++i;
    }
  }
}

// Function used with data types that are just received as BE/LE bytes
template <class Cursor, typename T, bool BE>
static inline void readUnencodedArithmeticVector(
    Cursor& c, T* outputPtr, size_t numElements) {
  size_t i = 0;
  while (i < numElements) {
    folly::ByteRange currentRange = c.peekBytes();
    const uint8_t* inPtr = currentRange.data();
    size_t loopBound =
        std::min(numElements, i + currentRange.size() / sizeof(T));
    size_t j = 0;
    for (; i < loopBound; ++i, ++j) {
      T value = BE ? folly::Endian::big<T>(
                         folly::loadUnaligned<T>(inPtr + j * sizeof(T)))
                   : folly::loadUnaligned<T>(inPtr + j * sizeof(T));
      outputPtr[i] = value;
    }
    c.skipNoAdvance(j * sizeof(T));
    if (i < numElements) {
      if constexpr (sizeof(T) == 8) {
        uint64_t bits = c.template readBE<int64_t>();
        outputPtr[i] = folly::bit_cast<double>(bits);
      } else if constexpr (sizeof(T) == 4) {
        uint32_t bits = c.template readBE<int32_t>();
        outputPtr[i] = folly::bit_cast<float>(bits);
      } else {
        outputPtr[i] = c.template read<int8_t>();
      }
      ++i;
    }
  }
}

template <>
void CompactProtocolReader::readArithmeticVector<int64_t>(
    int64_t* outputPtr, size_t numElements) {
  return readEncodedArithmeticVector<Cursor, int64_t>(
      in_, outputPtr, numElements);
}
template <>
void CompactProtocolReader::readArithmeticVector<uint64_t>(
    uint64_t* outputPtr, size_t numElements) {
  return readEncodedArithmeticVector<Cursor, int64_t>(
      in_, reinterpret_cast<int64_t*>(outputPtr), numElements);
}
template <>
void CompactProtocolReader::readArithmeticVector<int32_t>(
    int32_t* outputPtr, size_t numElements) {
  return readEncodedArithmeticVector<Cursor, int32_t>(
      in_, outputPtr, numElements);
}
template <>
void CompactProtocolReader::readArithmeticVector<uint32_t>(
    uint32_t* outputPtr, size_t numElements) {
  return readEncodedArithmeticVector<Cursor, int32_t>(
      in_, reinterpret_cast<int32_t*>(outputPtr), numElements);
}
template <>
void CompactProtocolReader::readArithmeticVector<int16_t>(
    int16_t* outputPtr, size_t numElements) {
  return readEncodedArithmeticVector<Cursor, int16_t>(
      in_, outputPtr, numElements);
}
template <>
void CompactProtocolReader::readArithmeticVector<uint16_t>(
    uint16_t* outputPtr, size_t numElements) {
  return readEncodedArithmeticVector<Cursor, int16_t>(
      in_, reinterpret_cast<int16_t*>(outputPtr), numElements);
}
template <>
void CompactProtocolReader::readArithmeticVector<int8_t>(
    int8_t* outputPtr, size_t numElements) {
  return readUnencodedArithmeticVector<Cursor, int8_t, false>(
      in_, outputPtr, numElements);
}
template <>
void CompactProtocolReader::readArithmeticVector<uint8_t>(
    uint8_t* outputPtr, size_t numElements) {
  return readUnencodedArithmeticVector<Cursor, uint8_t, false>(
      in_, outputPtr, numElements);
}
template <>
void CompactProtocolReader::readArithmeticVector<float>(
    float* outputPtr, size_t numElements) {
  return readUnencodedArithmeticVector<Cursor, float, true>(
      in_, outputPtr, numElements);
}
template <>
void CompactProtocolReader::readArithmeticVector<double>(
    double* outputPtr, size_t numElements) {
  return readUnencodedArithmeticVector<Cursor, double, true>(
      in_, outputPtr, numElements);
}

} // namespace apache::thrift
