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

#include "FdSocket.h"

namespace apache {
namespace thrift {
namespace rocket {

void writeChainWithFds(
    folly::AsyncTransport* transport,
    folly::AsyncTransport::WriteCallback* callback,
    std::unique_ptr<folly::IOBuf> buf,
    folly::SocketFds fds) {
  auto fdSocket = transport->getUnderlyingTransport<folly::AsyncFdSocket>();
  if (!fdSocket) {
    LOG(DFATAL) << "`writeChainWithFds` called when the underlying socket "
                << "was not a `AsyncFdSocket`: " << transport;
    // Fail back to sending data without FDs, and discard the FDs right
    // away.  The recipient will see a request that needs FDs without the
    // corresponding FDs on the wire, and fail out.
    transport->writeChain(callback, std::move(buf));
    return;
  }
  fdSocket->writeChainWithFds(callback, std::move(buf), std::move(fds));
}

} // namespace rocket
} // namespace thrift
} // namespace apache
