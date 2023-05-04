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

#include <folly/Conv.h>
#include <folly/File.h>
#include <folly/String.h>

#include "FdSocketMetadata.h"

namespace apache {
namespace thrift {
namespace rocket {

folly::SocketFds releaseFdsFromMetadata(RequestRpcMetadata& metadata) {
  if (!metadata.otherMetadata()) {
    return folly::SocketFds{};
  }
  auto& otherMetadata = *metadata.otherMetadata();
  auto it = otherMetadata.find("__UNSAFE_FDS_FOR_REQUEST__");
  if (it == otherMetadata.end()) {
    return folly::SocketFds{};
  }

  folly::SocketFds::ToSend fds;
  std::vector<folly::StringPiece> fdStrVec;
  folly::split(",", it->second, fdStrVec);
  for (const auto& fdStr : fdStrVec) {
    fds.emplace_back(
        std::make_shared<folly::File>(folly::to<int>(fdStr), /*ownsFd*/ false));
  }
  otherMetadata.erase(it);

  return fds.size() ? folly::SocketFds{std::move(fds)} : folly::SocketFds{};
}

} // namespace rocket
} // namespace thrift
} // namespace apache
