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
#include "hphp/runtime/base/type-string.h"

#include "hphp/util/blob-encoder.h"
#include "hphp/util/blob-writer.h"

#include <folly/testing/TestUtil.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <limits>
#include <iostream>
#include <map>
#include <stdexcept>
#include <utility>

namespace HPHP {

namespace {

enum class TestChunk {
  Data,
};

enum class TestIndex {
  Items,
};

const Blob::Magic kTestMagic = {'t', 'e', 's', 't'};

} // namespace

template<class T>
void testSerializationExactEquality(const T& val) {
  BlobEncoder encoder;
  T decodedVal;

  encoder(val);

  BlobDecoder decoder(encoder.data(), encoder.size());

  decoder(decodedVal);
  decoder.assertDone();

  EXPECT_EQ(decodedVal, val);
}

template <typename T>
struct IntegerSerializationTest : ::testing::Test {};

using IntImplementations = ::testing::Types<
                                            int8_t,
                                            uint8_t,
                                            int16_t,
                                            uint16_t,
                                            int32_t,
                                            uint32_t,
                                            int64_t,
                                            uint64_t
                                            >;

TYPED_TEST_CASE(IntegerSerializationTest,IntImplementations);

TYPED_TEST(IntegerSerializationTest, DoTest) {
  testSerializationExactEquality(std::numeric_limits<TypeParam>::min());
  testSerializationExactEquality(std::numeric_limits<TypeParam>::max());
  testSerializationExactEquality(TypeParam());
  for (int i = 0; i < std::numeric_limits<TypeParam>::digits; i ++) {
    testSerializationExactEquality(TypeParam(1) << i);
    testSerializationExactEquality(~(TypeParam(1) << i));
  }
}

TEST(BlobHelperTest, TestInputs) {
  testSerializationExactEquality(true);
  testSerializationExactEquality(false);
  testSerializationExactEquality(std::make_pair(false, 1));
  testSerializationExactEquality(std::make_pair(0xdeadbeef, 0xfaceb00c));

  testSerializationExactEquality((const StringData*) nullptr);
  testSerializationExactEquality(const_cast<const StringData*>(staticEmptyString()));
  const auto& heyo = makeStaticString("heyo");
  testSerializationExactEquality(const_cast<const StringData*>(heyo));
}

TEST(BlobHelperTest, WriterRejectsLivePartFile) {
  folly::test::TemporaryDirectory temp{"blob-writer-exclusive-part"};
  auto const path = (temp.path() / "test.blob").native();

  Blob::Writer<TestChunk, TestIndex> writer;
  writer.exclusiveHeader(path, kTestMagic, 1, Blob::ErrorMode::Throw);

  Blob::Writer<TestChunk, TestIndex> duplicate;
  EXPECT_THROW(
    duplicate.exclusiveHeader(path, kTestMagic, 1, Blob::ErrorMode::Throw),
    std::runtime_error
  );

  EXPECT_FALSE(std::filesystem::exists(path));
  EXPECT_TRUE(std::filesystem::exists(path + ".part"));

  writer.finish();

  EXPECT_TRUE(std::filesystem::exists(path));
  EXPECT_FALSE(std::filesystem::exists(path + ".part"));
}

TEST(BlobHelperTest, WriterHeaderTruncatesStalePartFile) {
  folly::test::TemporaryDirectory temp{"blob-writer-stale-part"};
  auto const path = (temp.path() / "test.blob").native();
  {
    std::ofstream part{path + ".part"};
    part << "stale";
  }

  Blob::Writer<TestChunk, TestIndex> writer;
  writer.header(path, kTestMagic, 1, Blob::ErrorMode::Throw);

  EXPECT_FALSE(std::filesystem::exists(path));
  EXPECT_TRUE(std::filesystem::exists(path + ".part"));

  writer.finish();

  EXPECT_TRUE(std::filesystem::exists(path));
  EXPECT_FALSE(std::filesystem::exists(path + ".part"));
}

TEST(BlobHelperTest, MMapReadRejectsOverflowingBounds) {
  folly::test::TemporaryFile temp{"blob-reader-overflow"};
  auto const path = temp.path().native();
  {
    std::ofstream file{path};
    file << "data";
  }

  Blob::FD fd{path, O_RDONLY, Blob::ErrorMode::Throw};
  fd.enableMmap(4);

  EXPECT_THROW(
    fd.readBlob(std::numeric_limits<size_t>::max() - 1, 4),
    std::runtime_error
  );
}

TEST(BlobHelperTest, ReaderThrowsOnCorruptHashMapIndexOffsets) {
  folly::test::TemporaryDirectory temp{"blob-reader-corrupt-index"};
  auto const path = (temp.path() / "test.blob").native();

  // Build a valid single-entry hash-map index.
  {
    Blob::Writer<TestChunk, TestIndex> writer;
    writer.header(path, kTestMagic, 1, Blob::ErrorMode::Throw);
    std::map<std::string, std::string> entries{{"k", "v"}};
    writer.hashMapIndex<std::string, Blob::CaseSensitiveCompare>(
      TestIndex::Items,
      entries,
      [](auto const& it) { return it.first; },
      [](auto const& it) { return &it.second; }
    );
    writer.finish();
  }

  size_t indexOffset;
  {
    Blob::Reader<TestChunk, TestIndex> reader;
    reader.init(
      path, kTestMagic, 1, Blob::ReadMode::PReadOnly, Blob::ErrorMode::Throw
    );
    indexOffset =
      reader.hashMapIndex<Blob::CaseSensitiveCompare>(TestIndex::Items)
        .indexBounds.offset;
  }

  // Corrupt bucket 0's start offset so it exceeds the next offset. Without the
  // bounds check, nextOffset - currentOffset underflows into an unbounded read.
  {
    std::fstream file{path, std::ios::in | std::ios::out | std::ios::binary};
    const uint32_t poison = std::numeric_limits<uint32_t>::max();
    file.seekp(static_cast<std::streamoff>(indexOffset));
    file.write(reinterpret_cast<const char*>(&poison), sizeof(poison));
  }

  Blob::Reader<TestChunk, TestIndex> reader;
  reader.init(
    path, kTestMagic, 1, Blob::ReadMode::PReadOnly, Blob::ErrorMode::Throw
  );
  auto map = reader.hashMapIndex<Blob::CaseSensitiveCompare>(TestIndex::Items);
  EXPECT_THROW(
    (reader.getFromIndex<std::string, std::string, Blob::CaseSensitiveCompare>(
      map, std::string{"k"}
    )),
    std::runtime_error
  );
}

}
