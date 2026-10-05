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

#include "hphp/runtime/vm/jit/cprof-serde.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "hphp/runtime/vm/jit/cprof-record.h"
#include "hphp/util/sha1.h"

namespace HPHP::jit::cprof {
namespace {

constexpr auto kInvalidBytecodeOffset =
  static_cast<uint32_t>(std::numeric_limits<int32_t>::max());

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

ContProfProfileTranslation exampleBytecodeTranslation(
  uint32_t bytecodeOffset = 4
) {
  return {
    ContProfStartKind::Bytecode,
    bytecodeOffset,
    3,
    13,
  };
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
    exampleBytecodeTranslation(),
  };
  record.translations[0].localTypeGuards = {
    {0, KindOfInt64},
    {2, KindOfString},
  };
  record.translations[0].localPostConditions = {
    {0, true, kInvalidDataType},
    {1, true, KindOfInt64},
    {2, false, KindOfString},
  };
  record.translations.back().localTypeGuards = {
    {1, KindOfObject},
  };
  record.translations.back().incoming = {0, 1, 3};
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
  EXPECT_EQ(1, deserialized->translations[1].numEntryArgs());
  EXPECT_EQ(4, deserialized->translations[3].offset());
}

TEST(ContProfSerde, ProfileTranslationWireFormat) {
  auto const record = exampleProfileRecord();
  auto const funcKey = serializeContProfFuncKey(record.header.funcKey);
  ASSERT_TRUE(funcKey);
  auto const serialized = serializeContProfProfileRecord(record);
  ASSERT_TRUE(serialized);

  // Check field widths and order, including nonempty optional data.
  std::vector<uint8_t> expected;
  auto const append = [&](auto value) {
    auto const offset = expected.size();
    expected.resize(offset + sizeof(value));
    std::memcpy(expected.data() + offset, &value, sizeof(value));
  };
  auto const appendTranslation = [&](uint8_t kind, uint32_t start,
                                    uint32_t length, uint64_t count) {
    append(kind);
    append(start);
    append(length);
    append(count);
  };
  appendTranslation(1, 0, 2, uint64_t{1} << 40);
  append(size_t{2}); // local type guards
  append(uint32_t{0});
  append(static_cast<uint8_t>(KindOfInt64));
  append(uint32_t{2});
  append(static_cast<uint8_t>(KindOfString));
  append(size_t{0}); // incoming translations
  append(size_t{3}); // local postconditions
  append(uint32_t{0});
  append(uint8_t{1});
  append(static_cast<uint8_t>(kInvalidDataType));
  append(uint32_t{1});
  append(uint8_t{1});
  append(static_cast<uint8_t>(KindOfInt64));
  append(uint32_t{2});
  append(uint8_t{0});
  append(static_cast<uint8_t>(KindOfString));
  appendTranslation(1, 1, 3, 7);
  append(size_t{0}); // local type guards
  append(size_t{0}); // incoming translations
  append(size_t{0}); // local postconditions
  appendTranslation(2, 0, 1, 11);
  append(size_t{0}); // local type guards
  append(size_t{0}); // incoming translations
  append(size_t{0}); // local postconditions
  appendTranslation(3, 4, 3, 13);
  append(size_t{1}); // local type guards
  append(uint32_t{1});
  append(static_cast<uint8_t>(KindOfObject));
  append(size_t{3}); // incoming translations
  append(uint32_t{0});
  append(uint32_t{1});
  append(uint32_t{3});
  append(size_t{0}); // local postconditions

  auto const translationsOffset = sizeof(size_t) + funcKey->size() +
    sizeof(uint64_t) + sizeof(size_t);
  ASSERT_EQ(translationsOffset + expected.size(), serialized->size());
  std::vector<uint8_t> const actual{
    serialized->begin() + translationsOffset, serialized->end()
  };
  EXPECT_EQ(expected, actual);
}

TEST(ContProfSerde, RejectsInvalidProfileRecord) {
  auto record = exampleProfileRecord();
  record.header.capturedAtMs = 0;
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations.clear();
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations[0].startKind = static_cast<ContProfStartKind>(0xff);
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations[0].regionLength = 0;
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations[0].executionCount = 0;
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations[2].offsetOrNumEntryArgs = 1;
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations.back().offsetOrNumEntryArgs = 0;
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations.back().offsetOrNumEntryArgs = kInvalidBytecodeOffset - 1;
  EXPECT_TRUE(serializeContProfProfileRecord(record));

  record.translations.back().offsetOrNumEntryArgs = kInvalidBytecodeOffset;
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations.back().incoming = {
    static_cast<uint32_t>(record.translations.size()),
  };
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations.back().incoming = {0, 0};
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations.back().incoming = {1, 0};
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations.front().incoming = {0};
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations[0].localPostConditions[0].changed = false;
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations[0].localPostConditions[1].type = static_cast<DataType>(0);
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations[0].localPostConditions[1].localId =
    record.translations[0].localPostConditions[0].localId;
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  std::swap(
    record.translations[0].localPostConditions[0],
    record.translations[0].localPostConditions[1]
  );
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations = {exampleBytecodeTranslation()};
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations.push_back(record.translations.back());
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations.push_back(exampleBytecodeTranslation(3));
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations[0].localTypeGuards[0].type = kInvalidDataType;
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  record.translations[0].localTypeGuards[1].localId =
    record.translations[0].localTypeGuards[0].localId;
  EXPECT_FALSE(serializeContProfProfileRecord(record));

  record = exampleProfileRecord();
  std::swap(
    record.translations[0].localTypeGuards[0],
    record.translations[0].localTypeGuards[1]
  );
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

TEST(ContProfSerde, RejectsMalformedLocalTypeGuards) {
  auto const record = exampleProfileRecord();
  auto const encodedFuncKey = serializeContProfFuncKey(record.header.funcKey);
  ASSERT_TRUE(encodedFuncKey);

  auto const serialized = serializeContProfProfileRecord(record);
  ASSERT_TRUE(serialized);

  constexpr size_t encodedFuncKeyLengthSize = sizeof(size_t);
  constexpr size_t capturedAtMsSize = sizeof(uint64_t);
  constexpr size_t translationCountSize = sizeof(size_t);
  constexpr size_t guardCountOffset =
    sizeof(uint8_t) +
    sizeof(uint32_t) +
    sizeof(uint32_t) +
    sizeof(uint64_t);

  auto const firstTranslation = encodedFuncKeyLengthSize +
    encodedFuncKey->size() +
    capturedAtMsSize +
    translationCountSize;
  auto const guardCountPosition =
    firstTranslation + guardCountOffset;
  auto const firstGuardTypePosition =
    guardCountPosition +
    sizeof(size_t) +
    sizeof(uint32_t);

  ASSERT_LT(firstGuardTypePosition, serialized->size());

  auto invalidType = *serialized;
  invalidType[firstGuardTypePosition] = 0;
  EXPECT_FALSE(deserializeContProfProfileRecord(
    folly::ByteRange{invalidType.data(), invalidType.size()}
  ));

  auto invalidCount = *serialized;
  auto const tooManyGuards = std::numeric_limits<size_t>::max();
  std::memcpy(
    invalidCount.data() + guardCountPosition,
    &tooManyGuards,
    sizeof(tooManyGuards)
  );
  EXPECT_FALSE(deserializeContProfProfileRecord(
    folly::ByteRange{invalidCount.data(), invalidCount.size()}
  ));
}

TEST(ContProfSerde, RejectsMalformedIncomingTopology) {
  auto const record = exampleProfileRecord();
  auto const encodedFuncKey = serializeContProfFuncKey(record.header.funcKey);
  ASSERT_TRUE(encodedFuncKey);

  auto const serialized = serializeContProfProfileRecord(record);
  ASSERT_TRUE(serialized);

  constexpr size_t encodedFuncKeyLengthSize = sizeof(size_t);
  constexpr size_t capturedAtMsSize = sizeof(uint64_t);
  constexpr size_t translationCountSize = sizeof(size_t);
  constexpr size_t guardCountOffset =
    sizeof(uint8_t) +
    sizeof(uint32_t) +
    sizeof(uint32_t) +
    sizeof(uint64_t);
  constexpr size_t encodedGuardSize =
    sizeof(uint32_t) + sizeof(uint8_t);

  auto const firstTranslation = encodedFuncKeyLengthSize +
    encodedFuncKey->size() +
    capturedAtMsSize +
    translationCountSize;
  auto const incomingCountPosition =
    firstTranslation +
    guardCountOffset +
    sizeof(size_t) +
    record.translations.front().localTypeGuards.size() *
      encodedGuardSize;

  ASSERT_LE(
    incomingCountPosition + sizeof(size_t),
    serialized->size()
  );

  auto invalidCount = *serialized;
  auto const tooManyIncoming = std::numeric_limits<size_t>::max();
  std::memcpy(
    invalidCount.data() + incomingCountPosition,
    &tooManyIncoming,
    sizeof(tooManyIncoming)
  );
  EXPECT_FALSE(deserializeContProfProfileRecord(
    folly::ByteRange{invalidCount.data(), invalidCount.size()}
  ));
}

TEST(ContProfSerde, RejectsMalformedLocalPostConditions) {
  auto const record = exampleProfileRecord();
  auto const encodedFuncKey = serializeContProfFuncKey(record.header.funcKey);
  ASSERT_TRUE(encodedFuncKey);

  auto const serialized = serializeContProfProfileRecord(record);
  ASSERT_TRUE(serialized);

  constexpr size_t encodedFuncKeyLengthSize = sizeof(size_t);
  constexpr size_t capturedAtMsSize = sizeof(uint64_t);
  constexpr size_t translationCountSize = sizeof(size_t);
  constexpr size_t guardCountOffset =
    sizeof(uint8_t) +
    sizeof(uint32_t) +
    sizeof(uint32_t) +
    sizeof(uint64_t);
  constexpr size_t encodedGuardSize =
    sizeof(uint32_t) + sizeof(uint8_t);

  auto const firstTranslation = encodedFuncKeyLengthSize +
    encodedFuncKey->size() +
    capturedAtMsSize +
    translationCountSize;
  auto const incomingCountPosition =
    firstTranslation +
    guardCountOffset +
    sizeof(size_t) +
    record.translations.front().localTypeGuards.size() *
      encodedGuardSize;
  auto const postCountPosition =
    incomingCountPosition +
    sizeof(size_t) +
    record.translations.front().incoming.size() * sizeof(uint32_t);
  auto const firstPost = postCountPosition + sizeof(size_t);
  auto const firstChanged = firstPost + sizeof(uint32_t);
  auto const firstType = firstChanged + sizeof(uint8_t);

  ASSERT_LT(firstType, serialized->size());

  auto invalidCount = *serialized;
  auto const tooManyPosts = std::numeric_limits<size_t>::max();
  std::memcpy(
    invalidCount.data() + postCountPosition,
    &tooManyPosts,
    sizeof(tooManyPosts)
  );
  EXPECT_FALSE(deserializeContProfProfileRecord(
    folly::ByteRange{invalidCount.data(), invalidCount.size()}
  ));

  auto invalidChanged = *serialized;
  invalidChanged[firstChanged] = 2;
  EXPECT_FALSE(deserializeContProfProfileRecord(
    folly::ByteRange{invalidChanged.data(), invalidChanged.size()}
  ));

  auto invalidType = *serialized;
  invalidType[firstType] = 0;
  EXPECT_FALSE(deserializeContProfProfileRecord(
    folly::ByteRange{invalidType.data(), invalidType.size()}
  ));
}

TEST(ContProfSerde, RejectsMalformedTranslationStart) {
  ContProfProfileRecord record{};
  record.header = exampleRecordHeader();
  record.translations = {
    {ContProfStartKind::FuncEntry, 0, 1, 1},
    exampleBytecodeTranslation(),
  };

  auto const encodedFuncKey = serializeContProfFuncKey(record.header.funcKey);
  ASSERT_TRUE(encodedFuncKey);

  auto const serialized = serializeContProfProfileRecord(record);
  ASSERT_TRUE(serialized);

  constexpr size_t encodedFuncKeyLengthSize = sizeof(size_t);
  constexpr size_t capturedAtMsSize = sizeof(uint64_t);
  constexpr size_t translationCountSize = sizeof(size_t);
  constexpr size_t startKindSize = sizeof(uint8_t);
  constexpr size_t startSize = sizeof(uint32_t);
  constexpr size_t regionLengthSize = sizeof(uint32_t);
  constexpr size_t executionCountSize = sizeof(uint64_t);
  constexpr size_t localTypeGuardCountSize = sizeof(size_t);
  constexpr size_t incomingCountSize = sizeof(size_t);
  constexpr size_t localPostConditionCountSize = sizeof(size_t);
  constexpr size_t encodedTranslationSize =
    startKindSize +
    startSize +
    regionLengthSize +
    executionCountSize +
    localTypeGuardCountSize +
    incomingCountSize +
    localPostConditionCountSize;

  auto const firstTranslation = encodedFuncKeyLengthSize +
    encodedFuncKey->size() +
    capturedAtMsSize +
    translationCountSize;
  auto const bytecodeTranslation = firstTranslation + encodedTranslationSize;
  auto const bytecodeOffsetPosition = bytecodeTranslation + startKindSize;

  ASSERT_LE(bytecodeOffsetPosition + sizeof(uint32_t), serialized->size());

  auto unknownStartKind = *serialized;
  unknownStartKind[bytecodeTranslation] = 0xff;
  EXPECT_FALSE(deserializeContProfProfileRecord(
    folly::ByteRange{unknownStartKind.data(), unknownStartKind.size()}
  ));

  auto invalidNamedEntry = *serialized;
  invalidNamedEntry[bytecodeTranslation] =
    static_cast<uint8_t>(ContProfStartKind::NamedParamsFuncEntry);
  EXPECT_FALSE(deserializeContProfProfileRecord(
    folly::ByteRange{invalidNamedEntry.data(), invalidNamedEntry.size()}
  ));

  auto invalidBytecodeOffset = *serialized;
  std::memcpy(
    invalidBytecodeOffset.data() + bytecodeOffsetPosition,
    &kInvalidBytecodeOffset,
    sizeof(kInvalidBytecodeOffset)
  );
  EXPECT_FALSE(deserializeContProfProfileRecord(
    folly::ByteRange{
      invalidBytecodeOffset.data(),
      invalidBytecodeOffset.size(),
    }
  ));
}

}
}
