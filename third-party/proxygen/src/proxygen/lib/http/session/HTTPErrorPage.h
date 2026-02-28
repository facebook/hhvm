/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <proxygen/lib/http/HTTPHeaders.h>
#include <proxygen/lib/http/ProxygenErrorEnum.h>
#include <string>

namespace folly {
class IOBuf;
}

namespace proxygen {

/**
 * An HTTPErrorPage generates the content for a web page that
 * Proxygen can return in response to various error conditions.
 */
class HTTPErrorPage {
 public:
  struct Page {
    Page(const std::string& pageContentType,
         std::unique_ptr<folly::IOBuf> pageContent)
        : contentType(pageContentType), content(std::move(pageContent)) {
    }

    Page(std::string pageContentType,
         std::unique_ptr<folly::IOBuf> pageContent,
         HTTPHeaders pageHeaders)
        : contentType(std::move(pageContentType)),
          content(std::move(pageContent)),
          headers(std::move(pageHeaders)) {
    }

    Page(Page&& other) noexcept = default;

    const std::string contentType;
    std::unique_ptr<folly::IOBuf> content;
    HTTPHeaders headers;
  };

  virtual ~HTTPErrorPage() {
  }

  virtual Page generate(uint64_t requestID,
                        unsigned httpStatusCode,
                        const std::string& reason,
                        std::unique_ptr<folly::IOBuf> body,
                        const std::string& detailReason,
                        ProxygenError err) const = 0;
};

/**
 * Static error page generator.
 */
class HTTPStaticErrorPage : public HTTPErrorPage {
 public:
  explicit HTTPStaticErrorPage(
      std::unique_ptr<folly::IOBuf> content,
      const std::string& contentType = "text/html; charset=utf-8");

  Page generate(uint64_t requestID,
                unsigned httpStatusCode,
                const std::string& reason,
                std::unique_ptr<folly::IOBuf> body,
                const std::string& detailReason,
                ProxygenError err) const override;

 protected:
  std::unique_ptr<folly::IOBuf> content_;
  std::string contentType_;
};

} // namespace proxygen
