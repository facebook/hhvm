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

#include <string>

#include <folly/ScopeGuard.h>
#include <gtest/gtest.h>
#include <zstd.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/zstd-string-buffer.h"

namespace HPHP {

namespace {

// Decompress a zstd frame produced by ZStdStringBuffer. `expectedSize` is the
// known decompressed length (streamed frames carry no content-size header, so
// it cannot be read back from the frame).
std::string zstdDecompress(const OptString& compressed, size_t expectedSize) {
  std::string out;
  out.resize(expectedSize + 16);
  auto const ret = ZSTD_decompress(
    out.data(), out.size(), compressed.data(), compressed.size());
  EXPECT_FALSE(ZSTD_isError(ret)) << ZSTD_getErrorName(ret);
  out.resize(ret);
  return out;
}

// A vec of `n` sequential ints.
Variant makeBigValue(int n) {
  VecInit vi(n);
  for (int i = 0; i < n; i++) {
    vi.append(make_tv<KindOfInt64>(i));
  }
  return Variant{vi.toArray()};
}

// A vec of `n` identical ints (serializes to many MB of "i:7;", compresses tiny).
Variant makeRepetitiveValue(int n) {
  VecInit vi(n);
  for (int i = 0; i < n; i++) {
    vi.append(make_tv<KindOfInt64>(7));
  }
  return Variant{vi.toArray()};
}

// A single huge string value (not a container): serialize() emits its body as
// one `s:LEN:"...";` record via a single append — the large-single-append path.
Variant makeHugeString(size_t n) {
  std::string buf(n, '\0');
  for (size_t i = 0; i < n; i++) buf[i] = static_cast<char>('a' + (i % 26));
  return Variant{buf};  // Variant(const std::string&) copies via StringData::Make
}

// Serialize `v` with no size limit (ground-truth uncompressed bytes).
OptString serializeUncompressed(const Variant& v) {
  VariableSerializer vs(VariableSerializer::Type::Serialize);
  vs.setIgnoreStringSizeLimit();
  return vs.serialize(v, /*ret=*/true);
}

}  // namespace

//////////////////////////////////////////////////////////////////////////////
// ZStdStringBuffer unit behavior

TEST(ZStdStringBuffer, CompressRoundTripSmall) {
  ZStdStringBuffer buf{};
  std::string data = "hello, world";
  buf.append(data.data(), static_cast<int>(data.size()));
  auto const out = buf.detach();
  EXPECT_GT(out.size(), 0);
  EXPECT_EQ(zstdDecompress(out, data.size()), data);
}

TEST(ZStdStringBuffer, CompressEmptyInputEmitsValidFrame) {
  ZStdStringBuffer buf{};
  auto const out = buf.detach();
  EXPECT_GT(out.size(), 0);
  EXPECT_EQ(zstdDecompress(out, 0), std::string{});
}

TEST(ZStdStringBuffer, CompressMultiChunk) {
  ZStdStringBuffer buf{};
  const std::string chunk(64 * 1024, 'a');
  std::string expected;
  for (int i = 0; i < 8; i++) {  // 8 * 64 KiB = 512 KiB > 128 KiB threshold
    buf.append(chunk.data(), static_cast<int>(chunk.size()));
    expected += chunk;
  }
  auto const out = buf.detach();
  EXPECT_GT(out.size(), 0);
  EXPECT_LT(out.size(), expected.size());  // it actually compressed
  EXPECT_EQ(zstdDecompress(out, expected.size()), expected);
}

TEST(ZStdStringBuffer, CompressDifferentLevels) {
  for (int level : {1, 6, 22}) {
    ZStdStringBuffer buf{level};
    std::string data(8192, 'x');
    buf.append(data.data(), static_cast<int>(data.size()));
    auto const out = buf.detach();
    EXPECT_EQ(zstdDecompress(out, data.size()), data)
      << "round-trip failed at level " << level;
  }
}

//////////////////////////////////////////////////////////////////////////////
// OOM / serialization-size-limit reproduction at the VariableSerializer level.

TEST(VariableSerializerZStd, NaiveSerializeTripsLimitButStreamingPasses) {
  auto const value = makeBigValue(200000);  // ~1.8 MB serialized

  auto const expected = serializeUncompressed(value);
  ASSERT_GT(expected.size(), 1u << 20);  // comfortably exceeds the cap below

  // Restore the limit afterward so other tests are unaffected.
  auto& limit = VariableSerializer::serializationSizeLimit->value;
  auto const savedLimit = limit;
  SCOPE_EXIT { limit = savedLimit; };
  limit = 512 * 1024;  // 512 KiB: below the ~1.8 MB uncompressed serialization

  {
    VariableSerializer vs(VariableSerializer::Type::Serialize);
    EXPECT_THROW(
      vs.serialize(value, /*ret=*/true),
      StringBufferLimitException);
  }

  OptString compressed;
  {
    ZStdVariableSerializer vs(VariableSerializer::Type::Serialize);
    compressed = vs.serialize(value, /*ret=*/true);
  }
  ASSERT_FALSE(compressed.isNull());
  EXPECT_GT(compressed.size(), 0);
  EXPECT_LT(compressed.size(), 512u * 1024);

  auto const decompressed = zstdDecompress(compressed, expected.size());
  EXPECT_EQ(decompressed, std::string(expected.data(), expected.size()));

  VariableUnserializer vu(
    decompressed.data(), decompressed.size(),
    VariableUnserializer::Type::Serialize);
  auto const roundTripped = vu.unserialize();
  EXPECT_TRUE(same(roundTripped, value));
}

// Measured via mmUsage() (the StringData/ArrayData pool that exhausts); usage()
// would also include zstd's bounded, malloc'd context, so it is excluded here.
TEST(VariableSerializerZStd, StreamingBoundsPeakRequestMemory) {
  // ~8 MB serialized ("i:7;" x 2,000,000), compresses to a few KB.
  auto const value = makeRepetitiveValue(2000000);

  auto const baseline = tl_heap->getStats().mmUsage();

  int64_t naiveResident;
  {
    VariableSerializer vs(VariableSerializer::Type::Serialize);
    vs.setIgnoreStringSizeLimit();
    auto const s = vs.serialize(value, /*ret=*/true);  // multi-MB, held alive
    ASSERT_FALSE(s.isNull());
    naiveResident = tl_heap->getStats().mmUsage() - baseline;
  }  // s freed here

  int64_t streamResident;
  {
    ZStdVariableSerializer vs(VariableSerializer::Type::Serialize);
    auto const compressed = vs.serialize(value, /*ret=*/true);
    ASSERT_FALSE(compressed.isNull());
    streamResident = tl_heap->getStats().mmUsage() - baseline;
  }

  RecordProperty("naive_resident_bytes", naiveResident);
  RecordProperty("stream_resident_bytes", streamResident);

  EXPECT_GT(naiveResident, 4 * 1024 * 1024);      // naive: > 4 MB resident
  EXPECT_LT(streamResident, naiveResident / 8);   // streaming: >= 8x smaller
}

TEST(VariableSerializerZStd, MatchesUncompressedPlaintext) {
  auto const value = makeBigValue(50000);
  auto const expected = serializeUncompressed(value);

  ZStdVariableSerializer vs(VariableSerializer::Type::Serialize);
  auto const compressed = vs.serialize(value, /*ret=*/true);

  auto const decompressed = zstdDecompress(compressed, expected.size());
  EXPECT_EQ(decompressed, std::string(expected.data(), expected.size()));
}

// NOTE: this asserts correctness + limit-avoidance, NOT bounded peak memory. For
// a single huge string the staging buffer holds the entire body before it is
// compressed (the large-single-append limitation), so peak uncompressed
// footprint here is ~the string size, unlike StreamingBoundsPeakRequestMemory.
TEST(VariableSerializerZStd, HugeSingleStringTripsLimitButStreamingPasses) {
  auto const value = makeHugeString(16 * 1024 * 1024);  // 16 MiB string body

  auto const expected = serializeUncompressed(value);
  ASSERT_GT(expected.size(), 16u << 20);  // body + `s:LEN:"...";` framing

  // Model the production limit well below the uncompressed serialization.
  auto& limit = VariableSerializer::serializationSizeLimit->value;
  auto const savedLimit = limit;
  SCOPE_EXIT { limit = savedLimit; };
  limit = 1 << 20;  // 1 MiB

  {
    VariableSerializer vs(VariableSerializer::Type::Serialize);
    EXPECT_THROW(
      vs.serialize(value, /*ret=*/true),
      StringBufferLimitException);
  }

  OptString compressed;
  {
    ZStdVariableSerializer vs(VariableSerializer::Type::Serialize);
    compressed = vs.serialize(value, /*ret=*/true);
  }
  ASSERT_FALSE(compressed.isNull());
  EXPECT_GT(compressed.size(), 0);
  EXPECT_LT(compressed.size(), 1u << 20);

  auto const decompressed = zstdDecompress(compressed, expected.size());
  EXPECT_EQ(decompressed, std::string(expected.data(), expected.size()));

  VariableUnserializer vu(
    decompressed.data(), decompressed.size(),
    VariableUnserializer::Type::Serialize);
  EXPECT_TRUE(same(vu.unserialize(), value));
}

// The largest length unserialize() accepts is MaxSerializedStringSize - 1 (the
// cap is a `>=` check), so a string of exactly that length is the boundary case.
TEST(VariableSerializerZStd, RoundTripsStringAtMaxSerializedStringSize) {
  auto const limit = Cfg::ErrorHandling::MaxSerializedStringSize;
  auto const value = makeHugeString(limit - 1);
  auto const expected = serializeUncompressed(value);

  ZStdVariableSerializer vs(VariableSerializer::Type::Serialize);
  auto const compressed = vs.serialize(value, /*ret=*/true);
  ASSERT_FALSE(compressed.isNull());

  auto const decompressed = zstdDecompress(compressed, expected.size());
  EXPECT_EQ(decompressed, std::string(expected.data(), expected.size()));

  VariableUnserializer vu(
    decompressed.data(), decompressed.size(),
    VariableUnserializer::Type::Serialize);
  EXPECT_TRUE(same(vu.unserialize(), value));
}

}  // namespace HPHP
