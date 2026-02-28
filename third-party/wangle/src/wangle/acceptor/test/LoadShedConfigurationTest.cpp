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

#include <wangle/acceptor/LoadShedConfiguration.h>

#include <folly/portability/GTest.h>

using namespace wangle;

TEST(LoadShedConfigurationTest, TestSettersAndGetters) {
  LoadShedConfiguration lsc;

  lsc.setMemSoftLimitRatio(0.1);
  EXPECT_EQ(0.1, lsc.getMemSoftLimitRatio());

  lsc.setMemHardLimitRatio(0.2);
  EXPECT_EQ(0.2, lsc.getMemHardLimitRatio());

  lsc.setCpuSoftLimitRatio(0.3);
  EXPECT_EQ(0.3, lsc.getCpuSoftLimitRatio());

  lsc.setCpuHardLimitRatio(0.4);
  EXPECT_EQ(0.4, lsc.getCpuHardLimitRatio());

  EXPECT_EQ(0, lsc.getCpuUsageExceedWindowSize());
  lsc.setCpuUsageExceedWindowSize(12);
  EXPECT_EQ(12, lsc.getCpuUsageExceedWindowSize());

  lsc.setLoadUpdatePeriod(std::chrono::milliseconds(1200));
  EXPECT_EQ(std::chrono::milliseconds(1200), lsc.getLoadUpdatePeriod());

  LoadShedConfiguration::AddressSet addressSet = {
      folly::SocketAddress("127.0.0.1", 1100),
      folly::SocketAddress("127.0.0.2", 1200),
      folly::SocketAddress("127.0.0.3", 1300),
  };
  lsc.setAllowlistAddrs(addressSet);

  EXPECT_EQ(addressSet, lsc.getAllowlistAddrs());
  EXPECT_TRUE(lsc.isAllowlisted(folly::SocketAddress("127.0.0.1", 1100)));
  EXPECT_TRUE(lsc.isAllowlisted(folly::SocketAddress("127.0.0.2", 1200)));
  EXPECT_TRUE(lsc.isAllowlisted(folly::SocketAddress("127.0.0.3", 1300)));
  EXPECT_FALSE(lsc.isAllowlisted(folly::SocketAddress("127.0.0.4", 1400)));
  lsc.addAllowlistAddr(folly::StringPiece("127.0.0.4"));
  EXPECT_TRUE(lsc.isAllowlisted(folly::SocketAddress("127.0.0.4", 0)));

  LoadShedConfiguration::NetworkSet networkSet = {
      NetworkAddress(folly::SocketAddress("127.0.0.5", 1500), 28),
      NetworkAddress(folly::SocketAddress("127.0.0.6", 1600), 24),
  };
  lsc.setAllowlistNetworks(networkSet);
  EXPECT_EQ(networkSet, lsc.getAllowlistNetworks());
  EXPECT_TRUE(lsc.isAllowlisted(folly::SocketAddress("127.0.0.5", 1500)));
  EXPECT_TRUE(lsc.isAllowlisted(folly::SocketAddress("127.0.0.6", 1300)));
  EXPECT_FALSE(lsc.isAllowlisted(folly::SocketAddress("10.0.0.7", 1700)));
  lsc.addAllowlistAddr(folly::StringPiece("10.0.0.7/20"));
  EXPECT_TRUE(lsc.isAllowlisted(folly::SocketAddress("10.0.0.7", 0)));
}
