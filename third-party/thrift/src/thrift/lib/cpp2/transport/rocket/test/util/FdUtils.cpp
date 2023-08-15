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

#include <boost/regex.hpp>

#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>

#include "FdUtils.h"

namespace apache {
namespace thrift {
namespace rocket {
namespace test {

std::string abbrevStr(const std::string& s, size_t maxLen) {
  constexpr std::string_view infix = "...";
  CHECK_GE(maxLen, infix.size() + 2);
  if (s.size() <= maxLen) {
    return s;
  }
  size_t endLen = (maxLen - infix.size()) / 2;
  size_t startLen = maxLen - infix.size() - endLen; // Can be `endLen + 1`
  return s.substr(0, startLen) + infix.data() + s.substr(s.size() - endLen);
}

std::string commaSeparatedFds(const folly::SocketFds::ToSend& fds) {
  std::vector<std::string> fdStrs;
  for (const auto& fd : fds) {
    fdStrs.emplace_back(folly::to<std::string>(fd->fd()));
  }
  return folly::join(",", fdStrs);
}

void InterceptedAsyncFdSocket::writeChainWithFds(
    WriteCallback* callback,
    std::unique_ptr<folly::IOBuf> buf,
    folly::SocketFds inFds,
    folly::WriteFlags flags) {
  // Some of the tests create LARGE requests that should get split among
  // multiple fragments.  Make sure this happens (so we can be sure that
  // FD handling on fragmented requests is correct).
  for (auto br : *buf) {
    CHECK_LE(br.size(), apache::thrift::rocket::kMaxFragmentedPayloadSize);
  }

  // Deconstruct `inFds` and reconstruct a copy as `fds`. This is required to
  // make assertions against `sendFds`, which is inaccessible in `SocketFds`.
  auto [sendFds, fdSeqNum] = *inFds.releaseToSendAndSeqNum();
  folly::SocketFds fds{sendFds};
  fds.setFdSocketSeqNumOnce(fdSeqNum);

  auto [checkRe, checkFds] = popQueue();
  const auto fdStr = renderFds(sendFds);
  const auto checkFdStr = renderFds(checkFds);
  LOG(INFO) << "Writing: expected " << checkFdStr << " vs actual " << fdStr;
  CHECK_EQ(checkFdStr, fdStr);

  regexFromWriteChainWithFds_ = *checkRe;
  SCOPE_EXIT { regexFromWriteChainWithFds_.reset(); };
  return folly::AsyncFdSocket::writeChainWithFds(
      callback, std::move(buf), std::move(fds), flags);
}

void InterceptedAsyncFdSocket::writeChain(
    WriteCallback* callback,
    std::unique_ptr<folly::IOBuf>&& buf,
    folly::WriteFlags flags) {
  // Check the data regex in `writeChain` because the case when the a
  // batch ends with no-FD writes does not call `writeChainWithFds`.
  std::string checkRe;
  if (regexFromWriteChainWithFds_.has_value()) { // normal case
    checkRe = std::move(*regexFromWriteChainWithFds_);
  } else { // batch ends on writes without FDs
    auto [re, fds] = popQueue();
    checkRe = std::move(*re);
    CHECK(fds.empty()) << checkRe;
  }

  const auto bufStr = buf->to<std::string>();
  LOG(INFO) << "Writing: expected regex " << checkRe << " vs actual "
            << renderData(bufStr);
  // This is kind of slow at 400ms for 16MiB. Why Boost?
  //  - `std::regex` stack-overflows due to lack of possessive quantifiers
  //  - RE2 doesn't handle sized repetitions above 1000.
  EXPECT_TRUE(boost::regex_search(bufStr, boost::regex{checkRe}));

  return folly::AsyncFdSocket::writeChain(callback, std::move(buf), flags);
}

} // namespace test
} // namespace rocket
} // namespace thrift
} // namespace apache
