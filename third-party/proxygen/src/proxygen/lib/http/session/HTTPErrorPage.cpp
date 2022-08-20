/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HTTPErrorPage.h>

#include <folly/io/IOBuf.h>

using std::string;

namespace proxygen {

HTTPStaticErrorPage::HTTPStaticErrorPage(std::unique_ptr<folly::IOBuf> content,
                                         const string& contentType)
    : content_(std::move(content)), contentType_(contentType) {
}

HTTPErrorPage::Page HTTPStaticErrorPage::generate(
    uint64_t /*requestID*/,
    unsigned /*httpStatusCode*/,
    const std::string& /*reason*/,
    std::unique_ptr<folly::IOBuf> /*body*/,
    const std::string& /*detailReason*/) const {

  return HTTPErrorPage::Page(contentType_, content_->clone());
}

} // namespace proxygen
