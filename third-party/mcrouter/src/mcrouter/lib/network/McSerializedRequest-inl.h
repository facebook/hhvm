/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/fibers/FiberManager.h>
#include "mcrouter/lib/network/MessageHelpers.h"

namespace facebook {
namespace memcache {

namespace detail {

template <class Request>
typename std::enable_if<HasKeyTrait<Request>::value, uint64_t>::type getKeySize(
    const Request& req) {
  return req.key_ref()->size();
}

template <class Request>
typename std::enable_if<!HasKeyTrait<Request>::value, uint64_t>::type
getKeySize(const Request&) {
  return 0;
}

} // namespace detail

template <class Request>
McSerializedRequest::McSerializedRequest(
    const Request& req,
    size_t reqId,
    mc_protocol_t protocol,
    const CodecIdRange& compressionCodecs)
    : protocol_(protocol), typeId_(Request::typeId) {
  folly::fibers::runInMainContext([&] {
    switch (protocol_) {
      case mc_ascii_protocol:
        new (&asciiRequest_) AsciiSerializedRequest;
        if (detail::getKeySize(req) > MC_KEY_MAX_LEN_ASCII) {
          result_ = Result::BAD_KEY;
          return;
        }
        if (!asciiRequest_.prepare(req, iovsBegin_, iovsCount_)) {
          result_ = Result::ERROR;
        }
        break;
      case mc_caret_protocol:
        new (&caretRequest_) CaretSerializedMessage;
        if (detail::getKeySize(req) > MC_KEY_MAX_LEN_CARET) {
          result_ = Result::BAD_KEY;
          return;
        }
        if (!caretRequest_.prepare(
                req, reqId, compressionCodecs, iovsBegin_, iovsCount_)) {
          result_ = Result::ERROR;
        }
        break;
      default:
        checkLogic(
            false, "Used unsupported protocol! Value: {}", (int)protocol_);
        result_ = Result::ERROR;
        iovsCount_ = 0;
    }
  });
}

} // namespace memcache
} // namespace facebook
