/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/H3EarlyDataHandler.h>
#include <proxygen/lib/http/codec/HQUtils.h>

using namespace proxygen;

class H3EarlyDataHandlerTest : public ::testing::Test {
 protected:
  H3EarlyDataHandler handler_;

  HTTPSettings makeSettings(
      std::initializer_list<std::pair<SettingsId, uint64_t>> pairs) {
    HTTPSettings settings({});
    for (const auto& [id, val] : pairs) {
      settings.setSetting(id, val);
    }
    return settings;
  }
};

TEST_F(H3EarlyDataHandlerTest, GetAndValidateRoundtrip) {
  auto settings = makeSettings({{SettingsId::HEADER_TABLE_SIZE, 4096},
                                {SettingsId::_HQ_QPACK_BLOCKED_STREAMS, 100},
                                {SettingsId::MAX_HEADER_LIST_SIZE, 131072}});
  handler_.setCurrentSettings(settings);
  auto buf = handler_.get();
  ASSERT_NE(buf, nullptr);

  // Validate with same settings should succeed
  EXPECT_TRUE(handler_.validate(std::string("h3"), buf));
}

TEST_F(H3EarlyDataHandlerTest, ValidateWithLargerCurrentLimits) {
  // Serialize with small limits
  auto smallSettings =
      makeSettings({{SettingsId::HEADER_TABLE_SIZE, 2048},
                    {SettingsId::_HQ_QPACK_BLOCKED_STREAMS, 50}});
  handler_.setCurrentSettings(smallSettings);
  auto buf = handler_.get();
  ASSERT_NE(buf, nullptr);

  // Increase current limits — cached (smaller) values should still be valid
  auto largerSettings =
      makeSettings({{SettingsId::HEADER_TABLE_SIZE, 4096},
                    {SettingsId::_HQ_QPACK_BLOCKED_STREAMS, 100}});
  handler_.setCurrentSettings(largerSettings);
  EXPECT_TRUE(handler_.validate(std::string("h3"), buf));
}

TEST_F(H3EarlyDataHandlerTest, ValidateRejectsCachedHeaderTableSizeTooLarge) {
  auto settings = makeSettings({{SettingsId::HEADER_TABLE_SIZE, 8192}});
  handler_.setCurrentSettings(settings);
  auto buf = handler_.get();
  ASSERT_NE(buf, nullptr);

  // Lower current table size — cached value is too large
  auto smallerSettings = makeSettings({{SettingsId::HEADER_TABLE_SIZE, 4096}});
  handler_.setCurrentSettings(smallerSettings);
  EXPECT_FALSE(handler_.validate(std::string("h3"), buf));
}

TEST_F(H3EarlyDataHandlerTest, ValidateRejectsCachedBlockedStreamsTooLarge) {
  auto settings = makeSettings({{SettingsId::_HQ_QPACK_BLOCKED_STREAMS, 200}});
  handler_.setCurrentSettings(settings);
  auto buf = handler_.get();
  ASSERT_NE(buf, nullptr);

  auto smallerSettings =
      makeSettings({{SettingsId::_HQ_QPACK_BLOCKED_STREAMS, 100}});
  handler_.setCurrentSettings(smallerSettings);
  EXPECT_FALSE(handler_.validate(std::string("h3"), buf));
}

TEST_F(H3EarlyDataHandlerTest, ValidateRejectsCachedMaxHeaderListSizeTooLarge) {
  auto settings = makeSettings({{SettingsId::MAX_HEADER_LIST_SIZE, 262144}});
  handler_.setCurrentSettings(settings);
  auto buf = handler_.get();
  ASSERT_NE(buf, nullptr);

  auto smallerSettings =
      makeSettings({{SettingsId::MAX_HEADER_LIST_SIZE, 131072}});
  handler_.setCurrentSettings(smallerSettings);
  EXPECT_FALSE(handler_.validate(std::string("h3"), buf));
}

TEST_F(H3EarlyDataHandlerTest, ValidateWithZeroSettings) {
  auto settings = makeSettings({{SettingsId::HEADER_TABLE_SIZE, 0},
                                {SettingsId::_HQ_QPACK_BLOCKED_STREAMS, 0}});
  handler_.setCurrentSettings(settings);
  auto buf = handler_.get();
  ASSERT_NE(buf, nullptr);

  EXPECT_TRUE(handler_.validate(std::string("h3"), buf));
}

TEST_F(H3EarlyDataHandlerTest, ValidateWithNullAppParams) {
  auto settings = makeSettings({{SettingsId::HEADER_TABLE_SIZE, 4096}});
  handler_.setCurrentSettings(settings);
  EXPECT_FALSE(handler_.validate(std::string("h3"), nullptr));
}

TEST_F(H3EarlyDataHandlerTest, ValidateWithEmptyAppParams) {
  auto settings = makeSettings({{SettingsId::HEADER_TABLE_SIZE, 4096}});
  handler_.setCurrentSettings(settings);
  auto emptyBuf = folly::IOBuf::create(0);
  EXPECT_FALSE(handler_.validate(std::string("h3"), emptyBuf));
}

TEST_F(H3EarlyDataHandlerTest, ValidateWithTruncatedAppParams) {
  auto settings = makeSettings({{SettingsId::HEADER_TABLE_SIZE, 4096},
                                {SettingsId::_HQ_QPACK_BLOCKED_STREAMS, 100}});
  handler_.setCurrentSettings(settings);
  auto buf = handler_.get();
  ASSERT_NE(buf, nullptr);

  // Truncate buffer to just the count varint
  folly::io::Cursor cursor(buf.get());
  auto countVarint = quic::follyutils::decodeQuicInteger(cursor);
  ASSERT_TRUE(countVarint.has_value());
  auto truncatedBuf = folly::IOBuf::create(countVarint->second);
  memcpy(truncatedBuf->writableData(), buf->data(), countVarint->second);
  truncatedBuf->append(countVarint->second);

  EXPECT_FALSE(handler_.validate(std::string("h3"), truncatedBuf));
}

TEST_F(H3EarlyDataHandlerTest, ValidateWithNoAlpn) {
  auto settings = makeSettings({{SettingsId::HEADER_TABLE_SIZE, 4096}});
  handler_.setCurrentSettings(settings);
  auto buf = handler_.get();
  ASSERT_NE(buf, nullptr);

  // No ALPN — should still validate based on settings
  EXPECT_TRUE(handler_.validate(std::nullopt, buf));
}

TEST_F(H3EarlyDataHandlerTest, ValidateExactMatch) {
  auto settings = makeSettings({{SettingsId::HEADER_TABLE_SIZE, 4096},
                                {SettingsId::_HQ_QPACK_BLOCKED_STREAMS, 100},
                                {SettingsId::MAX_HEADER_LIST_SIZE, 131072}});
  handler_.setCurrentSettings(settings);
  auto buf = handler_.get();
  ASSERT_NE(buf, nullptr);

  // Exact same settings should validate
  handler_.setCurrentSettings(settings);
  EXPECT_TRUE(handler_.validate(std::string("h3"), buf));
}

TEST_F(H3EarlyDataHandlerTest, DefaultSettingsRoundtrip) {
  // Handler with no setCurrentSettings() should still work (empty settings)
  auto buf = handler_.get();
  ASSERT_NE(buf, nullptr);

  // Without setCurrentSettings(), validate takes the client-side path
  // (parse-only, no compatibility check). Verify it also works server-side.
  EXPECT_TRUE(handler_.validate(std::string("h3"), buf));

  // Now set empty settings and validate again — server-side path with
  // zero cached settings should also succeed.
  handler_.setCurrentSettings(makeSettings({}));
  buf = handler_.get();
  ASSERT_NE(buf, nullptr);
  EXPECT_TRUE(handler_.validate(std::string("h3"), buf));
}

TEST_F(H3EarlyDataHandlerTest, BooleanSettingCachedEnabledCurrentDisabled) {
  auto settings = makeSettings({{SettingsId::ENABLE_CONNECT_PROTOCOL, 1}});
  handler_.setCurrentSettings(settings);
  auto buf = handler_.get();
  ASSERT_NE(buf, nullptr);

  // Disable the setting — cached was enabled, so reject
  auto disabledSettings =
      makeSettings({{SettingsId::ENABLE_CONNECT_PROTOCOL, 0}});
  handler_.setCurrentSettings(disabledSettings);
  EXPECT_FALSE(handler_.validate(std::string("h3"), buf));
}

TEST_F(H3EarlyDataHandlerTest, BooleanSettingCachedDisabledCurrentEnabled) {
  auto settings = makeSettings({{SettingsId::ENABLE_CONNECT_PROTOCOL, 0}});
  handler_.setCurrentSettings(settings);
  auto buf = handler_.get();
  ASSERT_NE(buf, nullptr);

  // Enable the setting — cached was disabled, so this is fine
  auto enabledSettings =
      makeSettings({{SettingsId::ENABLE_CONNECT_PROTOCOL, 1}});
  handler_.setCurrentSettings(enabledSettings);
  EXPECT_TRUE(handler_.validate(std::string("h3"), buf));
}

TEST_F(H3EarlyDataHandlerTest, CachedNonDefaultSettingAbsentFromCurrent) {
  // Cache has a setting with non-default (non-zero) value
  auto settings = makeSettings({{SettingsId::HEADER_TABLE_SIZE, 4096}});
  handler_.setCurrentSettings(settings);
  auto buf = handler_.get();
  ASSERT_NE(buf, nullptr);

  // Current has no settings — cached non-default value is incompatible
  auto emptySettings = makeSettings({});
  handler_.setCurrentSettings(emptySettings);
  EXPECT_FALSE(handler_.validate(std::string("h3"), buf));
}

TEST_F(H3EarlyDataHandlerTest, CachedZeroSettingAbsentFromCurrent) {
  // Cache has a setting with default (zero) value
  auto settings = makeSettings({{SettingsId::HEADER_TABLE_SIZE, 0}});
  handler_.setCurrentSettings(settings);
  auto buf = handler_.get();
  ASSERT_NE(buf, nullptr);

  // Current has no settings — cached zero value is compatible
  auto emptySettings = makeSettings({});
  handler_.setCurrentSettings(emptySettings);
  EXPECT_TRUE(handler_.validate(std::string("h3"), buf));
}

TEST_F(H3EarlyDataHandlerTest, ClientSideValidateStoresSettings) {
  // Simulate server-side: serialize settings into a blob
  H3EarlyDataHandler serverHandler;
  serverHandler.setCurrentSettings(
      makeSettings({{SettingsId::HEADER_TABLE_SIZE, 4096},
                    {SettingsId::_HQ_QPACK_BLOCKED_STREAMS, 100},
                    {SettingsId::ENABLE_CONNECT_PROTOCOL, 1}}));
  auto buf = serverHandler.get();
  ASSERT_NE(buf, nullptr);

  // Client handler: no setCurrentSettings() called — hasSettings() is false
  H3EarlyDataHandler clientHandler;
  EXPECT_FALSE(clientHandler.hasSettings());

  // validate() in client mode parses the blob and stores settings
  EXPECT_TRUE(clientHandler.validate(std::string("h3"), buf));
  EXPECT_TRUE(clientHandler.hasSettings());

  // Verify parsed settings are accessible
  auto* tableSize =
      clientHandler.getSettings().getSetting(SettingsId::HEADER_TABLE_SIZE);
  ASSERT_NE(tableSize, nullptr);
  EXPECT_EQ(tableSize->value, 4096);

  auto* blocked = clientHandler.getSettings().getSetting(
      SettingsId::_HQ_QPACK_BLOCKED_STREAMS);
  ASSERT_NE(blocked, nullptr);
  EXPECT_EQ(blocked->value, 100);

  auto* connectProto = clientHandler.getSettings().getSetting(
      SettingsId::ENABLE_CONNECT_PROTOCOL);
  ASSERT_NE(connectProto, nullptr);
  EXPECT_EQ(connectProto->value, 1);
}

TEST_F(H3EarlyDataHandlerTest, SetCurrentSettingsFromSettingsList) {
  SettingsList settingsList = {{SettingsId::HEADER_TABLE_SIZE, 4096},
                               {SettingsId::_HQ_QPACK_BLOCKED_STREAMS, 100},
                               {SettingsId::ENABLE_CONNECT_PROTOCOL, 1},
                               // Include a non-HQ setting — should be filtered
                               {SettingsId::MAX_CONCURRENT_STREAMS, 200}};
  handler_.setCurrentSettings(settingsList);
  auto buf = handler_.get();
  ASSERT_NE(buf, nullptr);

  // Roundtrip should succeed
  EXPECT_TRUE(handler_.validate(std::string("h3"), buf));

  // Verify HQ settings were stored
  auto* tableSize =
      handler_.getSettings().getSetting(SettingsId::HEADER_TABLE_SIZE);
  ASSERT_NE(tableSize, nullptr);
  EXPECT_EQ(tableSize->value, 4096);

  // Non-HQ setting should have been filtered
  EXPECT_EQ(
      handler_.getSettings().getSetting(SettingsId::MAX_CONCURRENT_STREAMS),
      nullptr);
}

TEST_F(H3EarlyDataHandlerTest, ValidateIgnoresTrailingBytes) {
  handler_.setCurrentSettings(
      makeSettings({{SettingsId::HEADER_TABLE_SIZE, 4096}}));
  auto buf = handler_.get();
  ASSERT_NE(buf, nullptr);

  // Append trailing bytes to simulate a future format extension
  folly::IOBufQueue queue;
  queue.append(std::move(buf));
  auto extra = folly::IOBuf::copyBuffer("extra");
  queue.append(std::move(extra));
  auto bufWithTrailing = queue.move();

  EXPECT_TRUE(handler_.validate(std::string("h3"), bufWithTrailing));
}

TEST_F(H3EarlyDataHandlerTest, NonHQSettingsAreFiltered) {
  // Include non-HQ settings — they should be filtered out
  auto settings = makeSettings({{SettingsId::HEADER_TABLE_SIZE, 4096},
                                {SettingsId::ENABLE_PUSH, 1},
                                {SettingsId::MAX_CONCURRENT_STREAMS, 100},
                                {SettingsId::INITIAL_WINDOW_SIZE, 65535},
                                {SettingsId::MAX_FRAME_SIZE, 16384}});
  handler_.setCurrentSettings(settings);
  auto buf = handler_.get();
  ASSERT_NE(buf, nullptr);

  // Only HEADER_TABLE_SIZE should be in the serialized data
  EXPECT_TRUE(handler_.validate(std::string("h3"), buf));
}
