/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>

#include <gtest/gtest.h>

#include <folly/Random.h>
#include <folly/io/IOBuf.h>

#include "mcrouter/lib/CompressionCodecManager.h"
#include "mcrouter/lib/test/CompressionTestUtil.h"

namespace facebook {
namespace memcache {
namespace test {

namespace {

void validateCodec(CompressionCodec* codec) {
  EXPECT_TRUE(codec);

  auto data = createBinaryData(folly::Random::rand32(1, 16 * 1024));
  auto buf = folly::IOBuf::wrapBuffer(data.data(), data.size());
  auto compressedData = codec->compress(*buf);
  EXPECT_TRUE(compressedData);

  auto uncompressedData = codec->uncompress(*compressedData, data.size());
  EXPECT_EQ(data.size(), uncompressedData->computeChainDataLength());
  EXPECT_EQ(buf->coalesce(), uncompressedData->coalesce());
}

} // anonymous namespace

TEST(CompressionCodecManager, basic) {
  std::unordered_map<uint32_t, CodecConfigPtr> codecConfigs;
  for (uint32_t i = 1; i <= 64; ++i) {
    codecConfigs.emplace(
        i,
        std::make_unique<CodecConfig>(
            i, CompressionCodecType::LZ4, createBinaryData(i * 1024)));
  }

  CompressionCodecManager codecManager(std::move(codecConfigs));
  folly::EventBase evb;
  auto codecMap = codecManager.getCodecMap(evb);

  EXPECT_TRUE(codecMap);
  EXPECT_EQ(1, codecMap->getIdRange().firstId);
  EXPECT_EQ(64, codecMap->getIdRange().size);
  for (uint32_t i = 1; i <= 64; ++i) {
    validateCodec(codecMap->get(i));
  }
}

TEST(CompressionCodecManager, basicNotEnabledWithFilters) {
  constexpr size_t kTypeId = 1;
  std::unordered_map<uint32_t, CodecConfigPtr> codecConfigs;
  FilteringOptions filters;
  filters.isEnabled = false;
  filters.typeId = kTypeId;
  for (uint32_t i = 1; i <= 64; ++i) {
    codecConfigs.emplace(
        i,
        std::make_unique<CodecConfig>(
            i, CompressionCodecType::LZ4, createBinaryData(i * 1024), filters));
  }

  CompressionCodecManager codecManager(std::move(codecConfigs));
  folly::EventBase evb;
  auto codecMap = codecManager.getCodecMap(evb);

  EXPECT_TRUE(codecMap);
  EXPECT_EQ(1, codecMap->getIdRange().firstId);
  EXPECT_EQ(64, codecMap->getIdRange().size);
  size_t enabledCodecs = 0;
  size_t sameTypeId = 0;
  for (uint32_t i = 1; i <= 64; ++i) {
    auto codec = codecMap->get(i);
    if (codec->filteringOptions().isEnabled) {
      validateCodec(codec);
      enabledCodecs++;
    }
    if (codec->filteringOptions().typeId == kTypeId) {
      sameTypeId++;
    }
  }
  EXPECT_EQ(64, sameTypeId);
  EXPECT_EQ(0, enabledCodecs);
}

TEST(CompressionCodecManager, basicEnabled) {
  std::unordered_map<uint32_t, CodecConfigPtr> codecConfigs;
  for (uint32_t i = 1; i <= 64; ++i) {
    codecConfigs.emplace(
        i,
        std::make_unique<CodecConfig>(
            i,
            CompressionCodecType::LZ4,
            createBinaryData(i * 1024),
            FilteringOptions(),
            5 /* compression level */));
  }

  CompressionCodecManager codecManager(std::move(codecConfigs));
  folly::EventBase evb;
  auto codecMap = codecManager.getCodecMap(evb);

  EXPECT_TRUE(codecMap);
  EXPECT_EQ(1, codecMap->getIdRange().firstId);
  EXPECT_EQ(64, codecMap->getIdRange().size);
  size_t enabledCodecs = 0;
  for (uint32_t i = 1; i <= 64; ++i) {
    auto codec = codecMap->get(i);
    if (codec->filteringOptions().isEnabled) {
      validateCodec(codec);
      enabledCodecs++;
    }
  }
  EXPECT_EQ(64, enabledCodecs);
}

TEST(CompressionCodecManager, missingStart) {
  std::unordered_map<uint32_t, CodecConfigPtr> codecConfigs;
  for (uint32_t i = 10; i <= 64; ++i) {
    codecConfigs.emplace(
        i,
        std::make_unique<CodecConfig>(
            i, CompressionCodecType::LZ4, createBinaryData(i * 1024)));
  }

  CompressionCodecManager codecManager(std::move(codecConfigs));
  folly::EventBase evb;
  auto codecMap = codecManager.getCodecMap(evb);

  EXPECT_TRUE(codecMap);
  EXPECT_EQ(10, codecMap->getIdRange().firstId);
  EXPECT_EQ(55, codecMap->getIdRange().size);
  for (uint32_t i = 1; i <= 64; ++i) {
    if (i < 10) {
      EXPECT_FALSE(codecMap->get(i));
    } else {
      validateCodec(codecMap->get(i));
    }
  }
}

TEST(CompressionCodecManager, missingMiddle) {
  std::unordered_map<uint32_t, CodecConfigPtr> codecConfigs;
  for (uint32_t i = 1; i <= 20; ++i) {
    codecConfigs.emplace(
        i,
        std::make_unique<CodecConfig>(
            i, CompressionCodecType::LZ4, createBinaryData(i * 1024)));
  }
  for (uint32_t i = 50; i <= 64; ++i) {
    codecConfigs.emplace(
        i,
        std::make_unique<CodecConfig>(
            i, CompressionCodecType::LZ4, createBinaryData(i * 1024)));
  }

  CompressionCodecManager codecManager(std::move(codecConfigs));
  folly::EventBase evb;
  auto codecMap = codecManager.getCodecMap(evb);

  EXPECT_TRUE(codecMap);
  EXPECT_EQ(50, codecMap->getIdRange().firstId);
  EXPECT_EQ(15, codecMap->getIdRange().size);
  for (uint32_t i = 1; i <= 64; ++i) {
    if (i >= 50) {
      validateCodec(codecMap->get(i));
    } else {
      EXPECT_FALSE(codecMap->get(i));
    }
  }
}

TEST(CompressionCodecManager, missingEnd) {
  std::unordered_map<uint32_t, CodecConfigPtr> codecConfigs;
  for (uint32_t i = 1; i <= 50; ++i) {
    codecConfigs.emplace(
        i,
        std::make_unique<CodecConfig>(
            i, CompressionCodecType::LZ4, createBinaryData(i * 1024)));
  }

  CompressionCodecManager codecManager(std::move(codecConfigs));
  folly::EventBase evb;
  auto codecMap = codecManager.getCodecMap(evb);

  EXPECT_TRUE(codecMap);
  EXPECT_EQ(1, codecMap->getIdRange().firstId);
  EXPECT_EQ(50, codecMap->getIdRange().size);
  for (uint32_t i = 1; i <= 64; ++i) {
    if (i <= 50) {
      validateCodec(codecMap->get(i));
    } else {
      EXPECT_FALSE(codecMap->get(i));
    }
  }
}

TEST(CompressionCodecManager, invalidDictionary) {
  std::unordered_map<uint32_t, CodecConfigPtr> codecConfigs;
  codecConfigs.emplace(
      1,
      std::make_unique<CodecConfig>(
          1, CompressionCodecType::LZ4, createBinaryData(10 * 1024)));
  codecConfigs.emplace(
      2,
      std::make_unique<CodecConfig>(
          2, CompressionCodecType::LZ4, createBinaryData(65 * 1024)));
  codecConfigs.emplace(
      3,
      std::make_unique<CodecConfig>(
          3, CompressionCodecType::LZ4, createBinaryData(64 * 1024)));

  CompressionCodecManager codecManager(std::move(codecConfigs));
  folly::EventBase evb;
  auto codecMap = codecManager.getCodecMap(evb);

  EXPECT_TRUE(codecMap);
  EXPECT_EQ(3, codecMap->getIdRange().firstId);
  EXPECT_EQ(1, codecMap->getIdRange().size);

  EXPECT_FALSE(codecMap->get(1));
  EXPECT_FALSE(codecMap->get(2));
  validateCodec(codecMap->get(3));
}

TEST(CompressionCodecManager, getBest_validateCodecs) {
  auto codecConfigs = testCodecConfigs();
  CompressionCodecManager codecManager(std::move(codecConfigs));
  folly::EventBase evb;
  auto codecMap = codecManager.getCodecMap(evb);

  EXPECT_TRUE(codecMap);
  EXPECT_EQ(1, codecMap->getIdRange().firstId);
  EXPECT_EQ(10, codecMap->getIdRange().size);
  for (uint32_t i = 1; i <= 10; ++i) {
    validateCodec(codecMap->get(i));
    EXPECT_EQ(codecMap->get(i)->id(), i);
  }
}

TEST(CompressionCodecManager, getBest_noMatches) {
  auto codecConfigs = testCodecConfigs();
  CompressionCodecManager codecManager(std::move(codecConfigs));
  folly::EventBase evb;
  auto codecMap = codecManager.getCodecMap(evb);
  // client doesn't have codecs
  EXPECT_EQ(
      nullptr,
      codecMap->getBest(
          CodecIdRange::Empty, 123 /* body size */, 2 /* reply type id */));
  // codecs 3-5 do not satisfy filters
  EXPECT_EQ(
      nullptr,
      codecMap->getBest(
          CodecIdRange{3, 3}, 123 /* body size */, 2 /* reply type id */));
  // codecs 3-10 do not satisfy filters
  EXPECT_EQ(
      nullptr,
      codecMap->getBest(
          CodecIdRange{3, 7}, 123 /* body size */, 1 /* reply type id */));
  // client codec id range is outside of server codec id range
  EXPECT_EQ(
      nullptr,
      codecMap->getBest(
          CodecIdRange{12, 7}, 123 /* body size */, 1 /* reply type id */));
  // client codec id range is larger than server codec id range
  EXPECT_EQ(
      nullptr,
      codecMap->getBest(
          CodecIdRange{8, 7}, 123 /* body size */, 1 /* reply type id */));
  // body size is less than all minCompressionThresholds
  EXPECT_EQ(
      nullptr,
      codecMap->getBest(
          CodecIdRange{1, 10}, 0 /* body size */, 1 /* reply type id */));
}

TEST(CompressionCodecManager, getBest_matches) {
  auto codecConfigs = testCodecConfigs();
  CompressionCodecManager codecManager(std::move(codecConfigs));
  folly::EventBase evb;
  auto codecMap = codecManager.getCodecMap(evb);
  EXPECT_EQ(
      codecMap->get(10),
      codecMap->getBest(
          CodecIdRange{2, 9}, 123 /* body size */, 2 /* reply type id */));
  EXPECT_EQ(
      codecMap->get(6),
      codecMap->getBest(
          CodecIdRange{1, 6}, 123 /* body size */, 2 /* reply type id */));
  EXPECT_EQ(
      codecMap->get(9),
      codecMap->getBest(
          CodecIdRange{1, 9}, 1234 /* body size */, 2 /* reply type id */));
  EXPECT_EQ(
      codecMap->get(8),
      codecMap->getBest(
          CodecIdRange{1, 10}, 1234 /* body size */, 1 /* reply type id */));
  // getting dictionary of type 0
  EXPECT_EQ(
      codecMap->get(2),
      codecMap->getBest(
          CodecIdRange{1, 10}, 123 /* body size */, 1 /* reply type id */));
  EXPECT_EQ(
      codecMap->get(2),
      codecMap->getBest(
          CodecIdRange{1, 10}, 123 /* body size */, 0 /* reply type id */));
  EXPECT_EQ(
      codecMap->get(4),
      codecMap->getBest(
          CodecIdRange{1, 10}, 1234 /* body size */, 0 /* reply type id */));
  // getting dictionary of type 0
  EXPECT_EQ(
      codecMap->get(4),
      codecMap->getBest(
          CodecIdRange{1, 10}, 1234 /* body size */, 19 /* reply type id */));
  // client codec id range is larger than server codec id range
  EXPECT_EQ(
      codecMap->get(2),
      codecMap->getBest(
          CodecIdRange{1, 15}, 123 /* body size */, 1 /* reply type id */));
}

TEST(CompressionCodecManager, getBest_serverWithoutCodecs) {
  std::unordered_map<uint32_t, CodecConfigPtr> codecConfigs;
  CompressionCodecManager codecManager(std::move(codecConfigs));
  folly::EventBase evb;
  auto codecMap = codecManager.getCodecMap(evb);
  EXPECT_EQ(
      nullptr,
      codecMap->getBest(
          CodecIdRange{2, 9}, 123 /* body size */, 2 /* reply type id */));
  EXPECT_EQ(
      nullptr,
      codecMap->getBest(
          CodecIdRange{1, 6}, 1234 /* body size */, 0 /* reply type id */));
}

} // namespace test
} // namespace memcache
} // namespace facebook
