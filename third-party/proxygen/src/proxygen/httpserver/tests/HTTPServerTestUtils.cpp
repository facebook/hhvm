/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/httpserver/tests/HTTPServerTestUtils.h>

#include <folly/portability/GTest.h>
#include <folly/portability/Sockets.h>
#include <folly/portability/Unistd.h>

namespace proxygen::test {

int getTcpMaxSegment(folly::NetworkSocket fd) {
  int mss = 0;
  socklen_t mssLength = sizeof(mss);
  EXPECT_EQ(::getsockopt(fd.toFd(), IPPROTO_TCP, TCP_MAXSEG, &mss, &mssLength),
            0);
  return mss;
}

int getDefaultLoopbackTcpMaxSegment() {
  auto fd = ::socket(AF_INET, SOCK_STREAM, 0);
  EXPECT_NE(fd, -1);
  const auto mss = getTcpMaxSegment(folly::NetworkSocket::fromFd(fd));
  EXPECT_EQ(::close(fd), 0);
  return mss;
}

int getDifferentTcpMaxSegment(int defaultTcpMaxSegment) {
  EXPECT_GT(defaultTcpMaxSegment, 100);
  return defaultTcpMaxSegment - 100;
}

} // namespace proxygen::test
