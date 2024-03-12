/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CompressionCodecManager.h"

#include <algorithm>

#include <folly/Format.h>
#include <folly/fibers/FiberManager.h>
#include <folly/io/IOBuf.h>

namespace facebook {
namespace memcache {

namespace {

bool isApplicable(const CompressionCodec* codec, const size_t bodySize) {
  if (codec == nullptr) {
    return false;
  }
  if (!codec->filteringOptions().isEnabled) {
    return false;
  }
  if (bodySize < codec->filteringOptions().minCompressionThreshold ||
      bodySize > codec->filteringOptions().maxCompressionThreshold) {
    return false;
  }
  return true;
}

} // anonymous namespace

/***************************
 * CompressionCodecManager *
 ***************************/
CompressionCodecManager::CompressionCodecManager(
    std::unordered_map<uint32_t, CodecConfigPtr> codecConfigs)
    : codecConfigs_(std::move(codecConfigs)) {
  // Validate all dictionaries
  std::vector<uint32_t> badCodecConfigs;
  int64_t largestId = 0;
  for (const auto& it : codecConfigs_) {
    auto codecId = it.first;
    const auto& config = it.second;
    try {
      // createCompressionCodec throws if the dictionary is invalid.
      createCompressionCodec(
          config->codecType,
          folly::IOBuf::wrapBuffer(
              config->dictionary.data(), config->dictionary.size()),
          codecId,
          config->filteringOptions,
          config->compressionLevel);
      largestId = std::max<int64_t>(largestId, codecId);
    } catch (const std::exception&) {
      badCodecConfigs.push_back(codecId);
      LOG(ERROR) << "Compression codec config [" << codecId << "] is invalid.";
    }
  }
  for (auto id : badCodecConfigs) {
    codecConfigs_.erase(id);
  }

  if (!codecConfigs_.empty()) {
    // Get the longest contiguous range ending in 'largestId'
    smallestCodecId_ = 0;
    for (int64_t i = largestId - 1; i >= 0; --i) {
      const auto& it = codecConfigs_.find(i);
      if (it == codecConfigs_.end()) {
        smallestCodecId_ = i + 1;
        break;
      }
    }
    size_ = largestId - smallestCodecId_ + 1;
    LOG(INFO) << "Using " << size_ << " compression codecs (range: ["
              << smallestCodecId_ << ", " << largestId << "])";
  } else {
    LOG(WARNING) << "No valid compression codec found. Compression disabled.";
  }
}

const CompressionCodecMap* CompressionCodecManager::getCodecMap(
    folly::EventBase& evb) const {
  const auto& map = compressionCodecMap_.try_emplace_with(evb, [this] {
    return folly::fibers::runInMainContext([this]() {
      if (size_ == 0) {
        return CompressionCodecMap{};
      }
      return CompressionCodecMap{codecConfigs_, smallestCodecId_, size_};
    });
  });
  return &map;
}

/****************
 * CodecIdRange *
 ****************/
const CodecIdRange CodecIdRange::Empty = CodecIdRange{1, 0};

/***********************
 * CompressionCodecMap *
 ***********************/
CompressionCodecMap::CompressionCodecMap() noexcept {}

CompressionCodecMap::CompressionCodecMap(
    const std::unordered_map<uint32_t, CodecConfigPtr>& codecConfigs,
    uint32_t smallestCodecId,
    uint32_t size)
    : firstId_(smallestCodecId) {
  assert(codecConfigs.size() >= size);

  codecs_.resize(size);
  size_t maxTypeId = 0;
  for (uint32_t id = firstId_; id < (firstId_ + size); ++id) {
    const auto& it = codecConfigs.find(id);
    CHECK(it != codecConfigs.end()) << "Dictionary " << id << " is missing!";
    const auto& config = it->second;
    maxTypeId = std::max(maxTypeId, config->filteringOptions.typeId);
    codecs_[index(id)] = createCompressionCodec(
        config->codecType,
        folly::IOBuf::wrapBuffer(
            config->dictionary.data(), config->dictionary.size()),
        id,
        config->filteringOptions,
        config->compressionLevel);
  }
  codecsIdByTypeId_.resize(maxTypeId + 1);
  for (uint32_t i = 0; i < size; ++i) {
    codecsIdByTypeId_[codecs_[i]->filteringOptions().typeId].push_back(
        codecs_[i]->id());
  }
}

CompressionCodec* CompressionCodecMap::get(uint32_t id) const noexcept {
  if (id < firstId_ || index(id) >= size()) {
    return nullptr;
  }
  return codecs_[index(id)].get();
}

CompressionCodec* CompressionCodecMap::getBestByTypeId(
    const CodecIdRange& codecRange,
    const size_t bodySize,
    const size_t typeId) const noexcept {
  if (typeId >= codecsIdByTypeId_.size()) {
    return nullptr;
  }
  uint32_t lastId = codecRange.firstId + codecRange.size - 1;
  for (int32_t i = codecsIdByTypeId_[typeId].size() - 1; i >= 0; --i) {
    if (codecsIdByTypeId_[typeId][i] < codecRange.firstId ||
        codecsIdByTypeId_[typeId][i] > lastId) {
      continue;
    }
    auto codec = get(codecsIdByTypeId_[typeId][i]);
    if (isApplicable(codec, bodySize)) {
      return codec;
    }
  }
  return nullptr;
}

CompressionCodec* CompressionCodecMap::getBest(
    const CodecIdRange& codecRange,
    const size_t bodySize,
    const size_t typeId) const noexcept {
  auto codec = getBestByTypeId(codecRange, bodySize, typeId);
  if (codec == nullptr) {
    codec = getBestByTypeId(codecRange, bodySize, 0 /* generic */);
  }
  return codec;
}

uint32_t CompressionCodecMap::index(uint32_t id) const noexcept {
  return id - firstId_;
}
} // namespace memcache
} // namespace facebook
