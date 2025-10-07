/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <fmt/format.h>
#include <glog/logging.h>
#include <proxygen/lib/utils/Export.h>

namespace proxygen {

// ParseURL can handle non-fully-formed URLs. This class must not persist beyond
// the lifetime of the buffer underlying the input StringPiece

class ParseURL {
 public:
  /* Parse a URL.  If parsing succeeds, return a fully formed ParseURL with
   * valid() == true.  If parsing fails, returns nothing. If you need the
   * partial parse results, use parseURLMaybeInvalid below.
   */
  static std::optional<ParseURL> parseURL(std::string_view urlVal,
                                          bool strict = false) noexcept {
    ParseURL parseUrl(urlVal, strict);
    if (parseUrl.valid()) {
      return parseUrl;
    }
    return std::nullopt;
  }

  /* Parse a URL.  Returns a ParseURL object that may or may not be valid.
   * Caller should check valid()
   */
  static ParseURL parseURLMaybeInvalid(std::string_view urlVal,
                                       bool strict = false) noexcept {
    return ParseURL(urlVal, strict);
  }

  static bool isSupportedScheme(std::string_view location);

  static std::optional<std::string> getRedirectDestination(
      std::string_view url,
      std::string_view requestScheme,
      std::string_view location,
      std::string_view headerHost) noexcept;

  // Deprecated.  Will be removed soon
  explicit ParseURL(std::string_view urlVal, bool strict = true) noexcept {
    init(urlVal, strict);
  }

  ParseURL(ParseURL&& goner) noexcept
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

  ParseURL& operator=(ParseURL&& goner) noexcept {
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

  void init(std::string_view urlVal, bool strict = false) {
    CHECK(!initialized_);
    url_ = urlVal;
    parse(strict);
    initialized_ = true;
  }

  operator bool() const {
    return valid();
  }

  [[nodiscard]] std::string_view url() const {
    return url_;
  }

  [[nodiscard]] std::string_view scheme() const {
    return scheme_;
  }

  [[nodiscard]] std::string authority() const {
    return authority_;
  }

  [[nodiscard]] bool hasHost() const {
    return valid() && !host_.empty();
  }

  [[nodiscard]] std::string_view host() const {
    return host_;
  }

  [[nodiscard]] uint16_t port() const {
    return port_;
  }

  [[nodiscard]] std::string hostAndPort() const {
    if (port_ == 0) {
      return std::string(host_);
    }
    return fmt::format("{}:{}", host_, port_);
  }

  [[nodiscard]] std::string_view path() const {
    return path_;
  }

  [[nodiscard]] std::string_view query() const {
    return query_;
  }

  [[nodiscard]] std::string_view fragment() const {
    return fragment_;
  }

  [[nodiscard]] bool valid() const {
    return valid_;
  }

  [[nodiscard]] std::string_view hostNoBrackets() {
    stripBrackets();
    return hostNoBrackets_;
  }

  [[nodiscard]] bool hostIsIPAddress();

  FB_EXPORT void stripBrackets() noexcept;

  [[nodiscard]] static std::optional<std::string_view> getQueryParam(
      std::string_view query, const std::string_view name) noexcept;

 private:
  void moveHostAndAuthority(ParseURL&& goner) noexcept {
    if (!valid_) {
      return;
    }
    int64_t hostOff = -1;
    int64_t hostNoBracketsOff = -1;
    const auto isFromUrl = [url = goner.url_](std::string_view s) {
      return s.data() >= url.data() && s.data() < url.data() + url.size();
    };
    if (goner.host_.empty() || isFromUrl(goner.host_)) {
      // relative url_
      host_ = goner.host_;
    } else {
      // relative authority_
      hostOff = goner.host_.data() - goner.authority_.data();
    }
    if (goner.hostNoBrackets_.empty() || isFromUrl(goner.hostNoBrackets_)) {
      // relative url_
      hostNoBrackets_ = goner.hostNoBrackets_;
    } else {
      // relative authority_
      hostNoBracketsOff =
          goner.hostNoBrackets_.data() - goner.authority_.data();
    }
    authority_ = std::move(goner.authority_);
    std::string_view authority(authority_);
    if (hostOff >= 0) {
      host_ = authority.substr(hostOff, goner.host_.size());
    }
    if (hostNoBracketsOff >= 0) {
      hostNoBrackets_ =
          authority.substr(hostNoBracketsOff, goner.hostNoBrackets_.size());
    }
  }

  FB_EXPORT void parse(bool strict) noexcept;

  void parseNonFully(bool strict) noexcept;

  bool parseAuthority() noexcept;

  std::string_view url_;
  std::string_view scheme_;
  std::string authority_;
  std::string_view host_;
  std::string_view hostNoBrackets_;
  std::string_view path_;
  std::string_view query_;
  std::string_view fragment_;
  uint16_t port_{0};
  bool valid_{false};
  bool initialized_{false};
};

} // namespace proxygen
