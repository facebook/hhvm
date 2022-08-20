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

#include <folly/net/NetOps.h>

#include <glog/logging.h>

#include <folly/net/NetworkSocket.h>
#include <folly/portability/GTest.h>

class NetOpsTest : public testing::Test {};

TEST_F(NetOpsTest, socketpair) {
  folly::NetworkSocket pair[2];
  PCHECK(0 == folly::netops::socketpair(AF_UNIX, SOCK_STREAM, 0, pair));
  std::string const textw = "hello world";
  PCHECK(
      long(textw.size()) ==
      folly::netops::send(pair[0], textw.data(), textw.size(), 0));
  std::string textr(100, '\0');
  PCHECK(
      long(textw.size()) ==
      folly::netops::recv(pair[1], &textr[0], textr.size(), 0));
  textr.resize(textw.size());
  EXPECT_EQ(textw, textr);
}
