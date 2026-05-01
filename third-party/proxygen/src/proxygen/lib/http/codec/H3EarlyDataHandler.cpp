/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/H3EarlyDataHandler.h>

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/logging/xlog.h>
#include <proxygen/lib/http/codec/HQUtils.h>
#include <quic/codec/QuicInteger.h>
#include <quic/folly_utils/Utils.h>

namespace proxygen {

namespace {

// Use a reasonable growth size to avoid a malloc per varint.
constexpr size_t kVarintBufGrowth = 128;

void writeVarint(folly::IOBufQueue& buf, uint64_t val) {
  folly::io::QueueAppender appender(&buf, kVarintBufGrowth);
  auto appenderOp = [appender = std::move(appender)](auto v) mutable {
    appender.writeBE(folly::tag<decltype(v)>, v);
  };
  auto res = quic::encodeQuicInteger(val, appenderOp);
  DCHECK(res.has_value());
}

auto readVarint(folly::io::Cursor& cursor) {
  return quic::follyutils::decodeQuicInteger(cursor);
}

// Returns true if the setting is a limit where cached <= current is required.
// Returns false for boolean/enable settings where cached enabled requires
// current enabled.
bool isLimitSetting(hq::SettingId id) {
  switch (id) {
    case hq::SettingId::HEADER_TABLE_SIZE:
    case hq::SettingId::QPACK_BLOCKED_STREAMS:
    case hq::SettingId::MAX_HEADER_LIST_SIZE:
    case hq::SettingId::H3_WT_MAX_SESSIONS:
    case hq::SettingId::WT_INITIAL_MAX_DATA:
      return true;
    case hq::SettingId::ENABLE_CONNECT_PROTOCOL:
    case hq::SettingId::H3_DATAGRAM:
    case hq::SettingId::H3_DATAGRAM_RFC:
    case hq::SettingId::ENABLE_WEBTRANSPORT:
    case hq::SettingId::WT_ENABLED:
      return false;
  }
  // Unknown settings are treated as limits
  return true;
}

} // namespace

void H3EarlyDataHandler::setCurrentSettings(HTTPSettings settings) {
  setCurrentSettings(settings.getAllSettings());
}

void H3EarlyDataHandler::setCurrentSettings(const SettingsList& settings) {
  settings_ = HTTPSettings({});
  for (const auto& setting : settings) {
    auto hqId = hq::httpToHqSettingsId(setting.id);
    if (hqId) {
      settings_.setSetting(setting.id, setting.value);
    }
  }
  settingsInitialized_ = true;
}

quic::BufPtr H3EarlyDataHandler::get() {
  auto& allSettings = settings_.getAllSettings();

  // Count only HQ-relevant settings
  uint64_t count = 0;
  for (const auto& setting : allSettings) {
    if (hq::httpToHqSettingsId(setting.id)) {
      ++count;
    }
  }

  XLOG(DBG4) << "H3EarlyDataHandler::get called, numSettings=" << count;

  folly::IOBufQueue buf{folly::IOBufQueue::cacheChainLength()};

  // Format: count, then (hqId, value) pairs as varints
  writeVarint(buf, count);
  for (const auto& setting : allSettings) {
    auto hqId = hq::httpToHqSettingsId(setting.id);
    if (hqId) {
      writeVarint(buf, static_cast<uint64_t>(*hqId));
      writeVarint(buf, setting.value);
    }
  }
  return buf.move();
}

bool H3EarlyDataHandler::validate(const quic::Optional<std::string>& /*alpn*/,
                                  const quic::BufPtr& appParams) {
  XLOG(DBG4) << "H3EarlyDataHandler::validate called, appParams="
             << (appParams ? "non-null" : "null")
             << " settingsInitialized=" << settingsInitialized_;
  if (!appParams) {
    return false;
  }

  folly::io::Cursor cursor(appParams.get());

  auto countResult = readVarint(cursor);
  if (!countResult) {
    XLOG(DBG2) << "Failed to decode settings count";
    return false;
  }
  auto count = countResult->first;

  // Client-side: parse cached settings and store them so they can be
  // applied at onTransportReady for 0-RTT.
  HTTPSettings cachedSettings({});

  for (uint64_t i = 0; i < count; ++i) {
    auto idResult = readVarint(cursor);
    if (!idResult) {
      XLOG(DBG2) << "Failed to decode setting id at index " << i;
      return false;
    }
    auto valResult = readVarint(cursor);
    if (!valResult) {
      XLOG(DBG2) << "Failed to decode setting value at index " << i;
      return false;
    }

    auto cachedHqId = static_cast<hq::SettingId>(idResult->first);
    auto cachedValue = valResult->first;

    // Store parsed setting for client-side use
    auto httpId = hq::hqToHttpSettingsId(cachedHqId);
    if (httpId) {
      cachedSettings.setSetting(*httpId, cachedValue);
    }

    // Client-side: settings not yet initialized. Just verify the blob is
    // parseable and store the cached settings for 0-RTT — the server does
    // the real compatibility check.
    if (!settingsInitialized_) {
      continue;
    }

    if (!httpId) {
      // Unknown setting in cache — skip (forward compatibility)
      continue;
    }

    const auto* currentSetting = settings_.getSetting(*httpId);
    if (!currentSetting) {
      // Setting was in cache but not in current settings.
      // Per RFC 9114, if the cached value is non-default (non-zero for H3
      // settings), this is incompatible.
      if (cachedValue != 0) {
        XLOG(DBG2) << "Cached setting " << idResult->first
                   << " has non-default value " << cachedValue
                   << " but is absent from current settings";
        return false;
      }
      continue;
    }

    if (isLimitSetting(cachedHqId)) {
      // For limits: cached must be <= current
      if (cachedValue > currentSetting->value) {
        XLOG(DBG2) << "Cached setting " << idResult->first << " value "
                   << cachedValue << " > current " << currentSetting->value;
        return false;
      }
    } else {
      // For boolean/enable: if cached was enabled, current must also be enabled
      if (cachedValue != 0 && currentSetting->value == 0) {
        XLOG(DBG2) << "Cached setting " << idResult->first
                   << " was enabled but current is disabled";
        return false;
      }
    }
  }

  if (!settingsInitialized_) {
    // Client-side: store the parsed cached settings so they can be applied
    // at onTransportReady for 0-RTT
    settings_ = std::move(cachedSettings);
    settingsInitialized_ = true;
  }

  // Trailing bytes after the declared count are ignored for forward
  // compatibility (e.g., future format extensions).
  return true;
}

} // namespace proxygen
