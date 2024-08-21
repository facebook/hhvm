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

#include <thrift/lib/cpp2/transport/rocket/PayloadUtils.h>

namespace apache::thrift::rocket {

class PayloadSerialzer {
 public:
  explicit PayloadSerialzer(folly::AsyncTransport* transport)
      : transport_(transport) {}

  template <typename Payload, typename Metadata>
  rocket::Payload serialize(
      Payload&& payload, Metadata* metadata, const RpcOptions& rpcOptions) {
    return serializeWithFds(
        std::forward<Payload>(payload),
        metadata,
        rpcOptions.copySocketFdsToSend());
  }

  template <typename Payload, typename Metadata>
  rocket::Payload serialize(
      Payload&& payload, Metadata* metadata, transport::THeader* headers) {
    return serializeWithFds(
        std::forward<Payload>(payload), metadata, std::move(headers->fds));
  }

  template <class T>
  rocket::Payload serialize(T&& payload) {
    auto metadata = std::forward<T>(payload).metadata;
    return serializeWithFds(
        std::forward<T>(payload).payload,
        &metadata,
        std::forward<T>(payload).fds);
  }

 private:
  template <typename Payload, typename Metadata>
  rocket::Payload serializeWithFds(
      Payload&& payload, Metadata* metadata, folly::SocketFds fds) {
    auto serializedPayload = packCompact(std::forward<Payload>(payload));
    if (auto compress = metadata->compression_ref()) {
      apache::thrift::rocket::detail::compressPayload(
          serializedPayload, *compress);
    }
    auto numFds = fds.size();
    if (numFds) {
      FdMetadata fdMetadata;

      // The kernel maximum is actually much lower (at least on Linux, and
      // MacOS doesn't seem to document it at all), but that will only fail in
      // in `AsyncFdSocket`.
      constexpr auto numFdsTypeMax = std::numeric_limits<
          op::get_native_type<FdMetadata, ident::numFds>>::max();
      if (UNLIKELY(numFds > numFdsTypeMax)) {
        LOG(DFATAL) << numFds << " would overflow FdMetadata::numFds";
        fdMetadata.numFds() = numFdsTypeMax;
        // This will cause "AsyncFdSocket::writeChainWithFds" to error out.
        fdMetadata.fdSeqNum() = folly::SocketFds::kNoSeqNum;
      } else {
        // When received, the request will know to retrieve this many FDs.
        fdMetadata.numFds() = numFds;
        // FD sequence numbers count the total number of FDs sent on this
        // socket, and are used to detect & fail on the dire class of bugs where
        // the wrong FDs are about to be associated with a message.
        //
        // We currently require message bytes and FDs to be both sent and
        // received in a coherent order, so sequence numbers here in `pack*` are
        // expected to exactly match the sequencing of socket sends, and also
        // the sequencing of `popNextReceivedFds` on the receiving side.
        //
        // NB: If `transport` is not backed by a `AsyncFdSocket*`, this will
        // store `fdSeqNum == -1`, which cannot happen otherwise, thanks to
        // AsyncFdSocket's 2^63 -> 0 wrap-around logic.  Furthermore, the
        // subsequent `writeChainWithFds` will discard `fds`.  As a result, the
        // recipient will see read errors on the FDs due to both `numFds` not
        // matching, and `fdSeqNum` not matching.
        fdMetadata.fdSeqNum() =
            injectFdSocketSeqNumIntoFdsToSend(transport_, &fds);
      }

      DCHECK(!metadata->fdMetadata().has_value());
      metadata->fdMetadata() = fdMetadata;
    }
    auto ret = apache::thrift::rocket::detail::makePayload(
        *metadata, std::move(serializedPayload));
    if (numFds) {
      ret.fds = std::move(fds.dcheckToSendOrEmpty());
    }
    return ret;
  }

  folly::AsyncTransport* transport_;
};

} // namespace apache::thrift::rocket
