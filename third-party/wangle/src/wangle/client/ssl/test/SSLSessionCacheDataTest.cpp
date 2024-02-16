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

#include <chrono>
#include <vector>

#include <folly/json/DynamicConverter.h>
#include <folly/portability/GTest.h>
#include <folly/ssl/OpenSSLPtrTypes.h>
#include <wangle/client/ssl/SSLSessionCacheData.h>
#include <wangle/client/ssl/SSLSessionCacheUtils.h>
#include <wangle/client/ssl/test/TestUtil.h>
#include <wangle/ssl/SSLUtil.h>

using namespace std::chrono;
using namespace testing;
using namespace wangle;
using folly::ssl::SSLSessionUniquePtr;

class SSLSessionCacheDataTest : public Test {
 public:
  void SetUp() override {
    sessions_ = getSessions();
  }

  void TearDown() override {
    for (const auto& it : sessions_) {
      SSL_SESSION_free(it.first);
    }
    sessions_.clear();
  }

 protected:
  std::vector<std::pair<SSL_SESSION*, size_t>> sessions_;
};

TEST_F(SSLSessionCacheDataTest, Basic) {
  SSLSessionCacheData data;
  data.sessionData = folly::fbstring("some session data");
  data.addedTime = system_clock::now();
  data.serviceIdentity = "some service";
  data.peerIdentities = "svc:serviceId";

  auto d = folly::toDynamic(data);
  auto deserializedData = folly::convertTo<SSLSessionCacheData>(d);

  EXPECT_EQ(deserializedData.sessionData, data.sessionData);
  EXPECT_EQ(deserializedData.addedTime, data.addedTime);
  EXPECT_EQ(deserializedData.serviceIdentity, data.serviceIdentity);
  EXPECT_EQ(deserializedData.peerIdentities, data.peerIdentities);
}

TEST_F(SSLSessionCacheDataTest, CloneSSLSession) {
  for (auto& it : sessions_) {
    auto sess = SSLSessionUniquePtr(cloneSSLSession(it.first));
    EXPECT_TRUE(sess);
  }
}

TEST_F(SSLSessionCacheDataTest, ServiceIdentity) {
  auto sessionPtr = SSLSessionUniquePtr(cloneSSLSession(sessions_[0].first));
  auto session = sessionPtr.get();
  auto ident = getSessionServiceIdentity(session);
  EXPECT_FALSE(ident);

  std::string id("serviceId");
  EXPECT_TRUE(setSessionServiceIdentity(session, id));
  ident = getSessionServiceIdentity(session);
  EXPECT_TRUE(ident);
  EXPECT_EQ(ident.value(), id);

  auto cloned = SSLSessionUniquePtr(cloneSSLSession(session));
  EXPECT_TRUE(cloned);
  ident = getSessionServiceIdentity(cloned.get());
  EXPECT_TRUE(ident);
  EXPECT_EQ(ident.value(), id);

  auto cacheDataOpt = getCacheDataForSession(session);
  EXPECT_TRUE(cacheDataOpt);
  auto& cacheData = cacheDataOpt.value();
  EXPECT_EQ(id, cacheData.serviceIdentity);

  auto deserialized = SSLSessionUniquePtr(getSessionFromCacheData(cacheData));
  EXPECT_TRUE(deserialized);
  ident = getSessionServiceIdentity(deserialized.get());
  EXPECT_TRUE(ident);
  EXPECT_EQ(ident.value(), id);
}

TEST_F(SSLSessionCacheDataTest, PeerIdentities) {
  std::string peerIdentities("svc:serviceId");
  auto sessionPtr = SSLSessionUniquePtr(cloneSSLSession(sessions_[0].first));
  auto session = sessionPtr.get();

  {
    auto identities = getSessionPeerIdentities(session);
    EXPECT_FALSE(identities.has_value());
  }

  auto success = setSessionPeerIdentities(session, peerIdentities);
  EXPECT_TRUE(success);

  {
    auto identities = getSessionPeerIdentities(session);
    EXPECT_TRUE(identities.has_value());
    EXPECT_EQ(identities.value(), peerIdentities);
  }

  {
    auto cloned = SSLSessionUniquePtr(cloneSSLSession(session));
    EXPECT_TRUE(cloned);
    auto identities = getSessionPeerIdentities(cloned.get());
    EXPECT_TRUE(identities.has_value());
    EXPECT_EQ(identities.value(), peerIdentities);
  }

  {
    auto cacheDataOpt = getCacheDataForSession(session);
    EXPECT_TRUE(cacheDataOpt.has_value());
    auto& cacheData = cacheDataOpt.value();
    EXPECT_EQ(peerIdentities, cacheData.peerIdentities);

    auto deserialized = SSLSessionUniquePtr(getSessionFromCacheData(cacheData));
    ASSERT_NE(deserialized, nullptr);
    auto identities = getSessionPeerIdentities(deserialized.get());
    EXPECT_TRUE(identities.has_value());
    EXPECT_EQ(identities.value(), peerIdentities);
  }
}
