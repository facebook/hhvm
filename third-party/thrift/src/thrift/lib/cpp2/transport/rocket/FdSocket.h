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

#pragma once

#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/fdsock/AsyncFdSocket.h>

namespace apache {
namespace thrift {
namespace rocket {

// When this is called, `transport` is actually expected to be an
// `AsyncFdSocket`, but we don't want to complicate the no-FDs, TCP-centric
// "fast path" by plumbing that through.  So, instead, let's `dynamic_cast`
// when it's needed ...  and cope with the consequences.
//
// KEEP THIS INVARIANT: For the receiver to correctly match FDs to a
// message, the FDs must be sent with the IOBuf ending with the FINAL
// fragment of that message.
void writeChainWithFds(
    folly::AsyncTransport* transport,
    folly::AsyncTransport::WriteCallback* callback,
    std::unique_ptr<folly::IOBuf> buf,
    folly::SocketFds fds);

} // namespace rocket
} // namespace thrift
} // namespace apache
