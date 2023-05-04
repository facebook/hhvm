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

#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/transport/rocket/FdSocketMetadata.h>

using namespace apache::thrift;

TEST(FdSocketMetadata, ReleaseFdsFromMetadataEmpty) {
  RequestRpcMetadata emptyMeta;
  EXPECT_TRUE(rocket::releaseFdsFromMetadata(emptyMeta).empty());
  EXPECT_EQ(RequestRpcMetadata{}, emptyMeta); // no changes

  RequestRpcMetadata noMagicKey;
  noMagicKey.otherMetadata() = {};
  EXPECT_TRUE(rocket::releaseFdsFromMetadata(noMagicKey).empty());
  RequestRpcMetadata alsoNoMagicKey;
  alsoNoMagicKey.otherMetadata() = {};
  EXPECT_EQ(alsoNoMagicKey, noMagicKey);
}

TEST(FdSocketMetadata, ReleaseFdsFromMetadata) {
  RequestRpcMetadata meta;
  meta.otherMetadata() = {
      {"__UNSAFE_FDS_FOR_REQUEST__", "2,0,1"},
      {"OtherKeysAreNotDeleted", "whew"},
  };
  auto fds = rocket::releaseFdsFromMetadata(meta);

  RequestRpcMetadata noMagicKey;
  noMagicKey.otherMetadata() = {{"OtherKeysAreNotDeleted", "whew"}};
  EXPECT_EQ(noMagicKey, meta);

  EXPECT_EQ(3, fds.size());
  auto sendFds = fds.releaseToSend();
  EXPECT_TRUE(fds.empty());
  EXPECT_EQ(3, sendFds.size());
  EXPECT_EQ(2, sendFds[0]->fd());
  EXPECT_EQ(0, sendFds[1]->fd());
  EXPECT_EQ(1, sendFds[2]->fd());
}
