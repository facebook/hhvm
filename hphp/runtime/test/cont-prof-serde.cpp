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

#include "hphp/runtime/vm/jit/cont-prof-serde.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "hphp/runtime/vm/jit/cont-prof-record.h"
#include "hphp/util/sha1.h"

namespace HPHP::jit {
namespace {

ContProfFuncKey exampleKey() {
  ContProfFuncKey key{};
  key.resolutionUnitPath = "src/example.php";
  key.bytecodeUnitHash = SHA1{uint64_t{1}};
  key.functionName = "example";
  return key;
}

void expectFunctionKeyRoundTrip(const ContProfFuncKey& expected) {
  auto const serialized = serializeContProfFuncKey(expected);
  ASSERT_TRUE(serialized);

  auto const deserialized = deserializeContProfFuncKey(
    folly::ByteRange{serialized->data(), serialized->size()}
  );
  ASSERT_TRUE(deserialized);
  EXPECT_EQ(expected, *deserialized);
}

ContProfRecordHeader exampleRecordHeader() {
  ContProfRecordHeader header{};
  header.funcKey = exampleKey();
  header.capturedAtMs = 1'700'000'000'000;
  return header;
}

ContProfProfileRecord exampleProfileRecord() {
  ContProfProfileRecord record{};
  record.header = exampleRecordHeader();
  record.translations = {
    {
      ContProfStartKind::FuncEntry,
      0,
      2,
      uint64_t{1} << 40,
    },
    {
      ContProfStartKind::FuncEntry,
      1,
      3,
      7,
    },
    {
      ContProfStartKind::NamedParamsFuncEntry,
      0,
      1,
      11,
    },
  };
  return record;
}

TEST(ContProfSerde, FunctionKeyRoundTrip) {
  expectFunctionKeyRoundTrip(exampleKey());

  auto key = exampleKey();
  key.bytecodeUnitPath = "src/bytecode.php";
  key.className = "ExampleClass";
  key.closureContextName = "ClosureContext";
  expectFunctionKeyRoundTrip(key);
}

TEST(ContProfSerde, RejectsInvalidFunctionKey) {
  auto key = exampleKey();
  key.bytecodeUnitHash = SHA1{};

  EXPECT_FALSE(serializeContProfFuncKey(key));
}

TEST(ContProfSerde, RejectsMalformedInput) {
  auto const serialized = serializeContProfFuncKey(exampleKey());
  ASSERT_TRUE(serialized);

  for (size_t size = 0; size < serialized->size(); ++size) {
    EXPECT_FALSE(deserializeContProfFuncKey(
      folly::ByteRange{serialized->data(), size}
    ));
  }

  auto trailingByte = *serialized;
  trailingByte.push_back(0);
  EXPECT_FALSE(deserializeContProfFuncKey(
    folly::ByteRange{trailingByte.data(), trailingByte.size()}
  ));

  auto wrongSchema = *serialized;
  auto const schemaOffset = sizeof(size_t);
  ASSERT_GT(wrongSchema.size(), schemaOffset);
  wrongSchema[schemaOffset] ^= 1;
  EXPECT_FALSE(deserializeContProfFuncKey(
    folly::ByteRange{wrongSchema.data(), wrongSchema.size()}
  ));
}

TEST(ContProfSerde, ProfileRecordRoundTrip) {
  auto const expected = exampleProfileRecord();
  auto const serialized = serializeContProfProfileRecord(expected);
  ASSERT_TRUE(serialized);

  auto const deserialized = deserializeContProfProfileRecord(
    folly::ByteRange{serialized->data(), serialized->size()}
  );
  ASSERT_TRUE(deserialized);
  EXPECT_EQ(expected, *deserialized);
  EXPECT_EQ((uint64_t{1} << 40) + 7 + 11, deserialized->functionExecutions());
}

TEST(ContProfSerde, RejectsInvalidProfileRecord) {
  auto record = exampleProfileRecord();
  record.header.capturedAtMs = 0;
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations.clear();
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations[0].startKind =
    static_cast<ContProfStartKind>(0xff);
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations[0].regionLength = 0;
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations[0].executionCount = 0;
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations.back().numEntryArgs = 1;
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  // Three individually valid counts overflow uint64_t to int64Max - 2.
  auto const int64Max = static_cast<uint64_t>(
    std::numeric_limits<int64_t>::max()
  );
  record = exampleProfileRecord();
  record.translations = {
    {ContProfStartKind::FuncEntry, 0, 1, int64Max},
    {ContProfStartKind::FuncEntry, 1, 1, int64Max},
    {ContProfStartKind::FuncEntry, 2, 1, int64Max},
  };
  EXPECT_FALSE(serializeContProfProfileRecord(record));
}

TEST(ContProfRecord, CanonicalTranslationOrder) {
  auto record = exampleProfileRecord();
  EXPECT_TRUE(record.hasCanonicalTranslationOrder());

  std::swap(record.translations[0], record.translations[1]);
  EXPECT_FALSE(record.hasCanonicalTranslationOrder());

  record = exampleProfileRecord();
  record.translations.insert(
    record.translations.begin() + 1,
    record.translations[0]
  );
  EXPECT_FALSE(record.hasCanonicalTranslationOrder());
}

TEST(ContProfSerde, RejectsMalformedProfileRecord) {
  auto const record = exampleProfileRecord();
  auto const serialized = serializeContProfProfileRecord(record);
  ASSERT_TRUE(serialized);

  for (size_t size = 0; size < serialized->size(); ++size) {
    EXPECT_FALSE(deserializeContProfProfileRecord(
      folly::ByteRange{serialized->data(), size}
    ));
  }

  auto trailingByte = *serialized;
  trailingByte.push_back(0);
  EXPECT_FALSE(deserializeContProfProfileRecord(
    folly::ByteRange{trailingByte.data(), trailingByte.size()}
  ));

  using EncodedFuncKeySize = size_t;
  using EncodedCapturedAt = uint64_t;

  auto corruptFuncKey = *serialized;
  auto const funcKeyOffset =
    sizeof(EncodedFuncKeySize) + sizeof(size_t);
  ASSERT_LT(funcKeyOffset, corruptFuncKey.size());
  corruptFuncKey[funcKeyOffset] ^= 1;
  EXPECT_FALSE(deserializeContProfProfileRecord(
    folly::ByteRange{corruptFuncKey.data(), corruptFuncKey.size()}
  ));

  auto const funcKey = serializeContProfFuncKey(record.header.funcKey);
  ASSERT_TRUE(funcKey);

  auto invalidTranslationCount = *serialized;
  auto const translationCountOffset =
    sizeof(EncodedFuncKeySize) + funcKey->size() +
    sizeof(EncodedCapturedAt);
  auto const translationCount = std::numeric_limits<size_t>::max();
  ASSERT_LE(
    translationCountOffset + sizeof(translationCount),
    invalidTranslationCount.size()
  );
  std::memcpy(
    invalidTranslationCount.data() + translationCountOffset,
    &translationCount,
    sizeof(translationCount)
  );
  EXPECT_FALSE(deserializeContProfProfileRecord(
    folly::ByteRange{
      invalidTranslationCount.data(),
      invalidTranslationCount.size()
    }
  ));
}

}
}
