/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <glog/logging.h>

#include <folly/Conv.h>
#include <folly/Optional.h>
#include <folly/Portability.h>
#include <folly/Range.h>
#include <folly/String.h>
#include <proxygen/lib/utils/Export.h>
#include <string>

namespace proxygen {

// ParseURL can handle non-fully-formed URLs. This class must not persist beyond
// the lifetime of the buffer underlying the input StringPiece

class ParseURL {
 public:
  /* Parse a URL.  If parsing succeeds, return a fully formed ParseURL with
   * valid() == true.  If parsing fails, returns nothing. If you need the
   * partial parse results, use parseURLMaybeInvalid below.
   */
  static folly::Expected<ParseURL, folly::Unit> parseURL(
      folly::StringPiece urlVal, bool strict = false) noexcept {
    ParseURL parseUrl(urlVal, strict);
    if (parseUrl.valid()) {
      return parseUrl;
    } else {
      return folly::makeUnexpected(folly::Unit());
    }
  }

  /* Parse a URL.  Returns a ParseURL object that may or may not be valid.
   * Caller should check valid()
   */
  static ParseURL parseURLMaybeInvalid(folly::StringPiece urlVal,
                                       bool strict = false) noexcept {
    return ParseURL(urlVal, strict);
  }

  // Deprecated.  Will be removed soon
  explicit ParseURL(folly::StringPiece urlVal, bool strict = true) noexcept {
    init(urlVal, strict);
  }

  ParseURL(ParseURL&& goner)
      : url_(goner.url_),
        scheme_(goner.scheme_),
        path_(goner.path_),
        query_(goner.query_),
        fragment_(goner.fragment_),
        port_(goner.port_),
        valid_(goner.valid_),
        initialized_(goner.initialized_) {
    moveHostAndAuthority(std::move(goner));
  }

  ParseURL& operator=(ParseURL&& goner) {
    url_ = goner.url_;
    scheme_ = goner.scheme_;
    path_ = goner.path_;
    query_ = goner.query_;
    fragment_ = goner.fragment_;
    port_ = goner.port_;
    valid_ = goner.valid_;
    initialized_ = goner.initialized_;
    moveHostAndAuthority(std::move(goner));
    return *this;
  }

  ParseURL& operator=(const ParseURL&) = delete;
  ParseURL(const ParseURL&) = delete;

  ParseURL() = default;

  void init(folly::StringPiece urlVal, bool strict = false) {
    CHECK(!initialized_);
    url_ = urlVal;
    parse(strict);
    initialized_ = true;
  }

  operator bool() const {
    return valid();
  }

  folly::StringPiece url() const {
    return url_;
  }

  folly::StringPiece scheme() const {
    return scheme_;
  }

  std::string authority() const {
    return authority_;
  }

  bool hasHost() const {
    return valid() && !host_.empty();
  }

  folly::StringPiece host() const {
    return host_;
  }

  uint16_t port() const {
    return port_;
  }

  std::string hostAndPort() const {
    std::string rc = host_.str();
    if (port_ != 0) {
      folly::toAppend(":", port_, &rc);
    }
    return rc;
  }

  folly::StringPiece path() const {
    return path_;
  }

  folly::StringPiece query() const {
    return query_;
  }

  folly::StringPiece fragment() const {
    return fragment_;
  }

  bool valid() const {
    return valid_;
  }

  folly::StringPiece hostNoBrackets() {
    stripBrackets();
    return hostNoBrackets_;
  }

  bool hostIsIPAddress();

  FB_EXPORT void stripBrackets() noexcept;

  FOLLY_NODISCARD folly::Optional<folly::StringPiece> getQueryParam(
      folly::StringPiece name) const noexcept;

 private:
  void moveHostAndAuthority(ParseURL&& goner) {
    if (!valid_) {
      return;
    }
    int64_t hostOff = -1;
    int64_t hostNoBracketsOff = -1;
    if (goner.host_.empty() || (goner.host_.data() >= goner.url_.data() &&
                                goner.host_.data() < goner.url_.end())) {
      // relative url_
      host_ = goner.host_;
    } else {
      // relative authority_
      hostOff = goner.host_.data() - goner.authority_.data();
    }
    if (goner.hostNoBrackets_.empty() ||
        (goner.hostNoBrackets_.data() >= goner.url_.data() &&
         goner.hostNoBrackets_.data() < goner.url_.end())) {
      // relative url_
      hostNoBrackets_ = goner.hostNoBrackets_;
    } else {
      // relative authority_
      hostNoBracketsOff =
          goner.hostNoBrackets_.data() - goner.authority_.data();
    }
    authority_ = std::move(goner.authority_);
    if (hostOff >= 0) {
      host_.reset(authority_.data() + hostOff, goner.host_.size());
    }
    if (hostNoBracketsOff >= 0) {
      hostNoBrackets_.reset(authority_.data() + hostNoBracketsOff,
                            goner.hostNoBrackets_.size());
    }
  }

  FB_EXPORT void parse(bool strict) noexcept;

  void parseNonFully(bool strict) noexcept;

  bool parseAuthority() noexcept;

  folly::StringPiece url_;
  folly::StringPiece scheme_;
  std::string authority_;
  folly::StringPiece host_;
  folly::StringPiece hostNoBrackets_;
  folly::StringPiece path_;
  folly::StringPiece query_;
  folly::StringPiece fragment_;
  uint16_t port_{0};
  bool valid_{false};
  bool initialized_{false};
};

} // namespace proxygen
