/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#include <folly/String.h>
#include <folly/portability/Windows.h> // for windows compatibility: STRICT maybe defined by some win headers
#include <proxygen/lib/utils/ParseURL.h>

#ifdef STRICT
#undef STRICT
#endif

#ifdef STRICT_COMPAT
#undef STRICT_COMPAT
#endif

namespace proxygen {

/**
 * Struct representing a URL.
 */

class URL {
 public:
  URL() = default;

  enum class Mode { STRICT_COMPAT, STRICT };
  explicit URL(std::string_view url,
               bool secure = false,
               Mode strict = Mode::STRICT_COMPAT) noexcept
      : URL(ParseURL::parseURLMaybeInvalid(url, strict == Mode::STRICT),
            secure,
            strict) {
  }

  explicit URL(ParseURL parseUrl,
               bool secure = false,
               Mode strict = Mode::STRICT_COMPAT) noexcept
      : host_(parseUrl.hostNoBrackets()),
        path_(parseUrl.path()),
        query_(parseUrl.query()),
        fragment_(parseUrl.fragment()),
        url_(parseUrl.url()) {
    setScheme(std::string(parseUrl.scheme()), secure);

    if (parseUrl.port()) {
      port_ = parseUrl.port();
    } else {
      port_ = isSecure() ? 443 : 80;
    }

    if (strict == Mode::STRICT) {
      valid_ &= parseUrl.valid();
    }
    // TODO: In STRICT_COMPAT, parseUrl.valid() is not checked, so URL.valid()
    // can be true so long as the scheme is http(s).
  }

  static std::string createUrl(std::string_view scheme,
                               std::string_view hostAndPort,
                               std::string_view path,
                               std::string_view query,
                               std::string_view fragment) noexcept {
    std::ostringstream out;
    out << scheme << "://" << hostAndPort << '/' << path;
    if (!query.empty()) {
      out << '?' << query;
    }
    if (!fragment.empty()) {
      out << '#' << fragment;
    }
    return out.str();
  }

  URL(std::string scheme,
      std::string host,
      uint16_t port = 0,
      std::string path = "",
      std::string query = "",
      std::string fragment = "") noexcept
      : host_(std::move(host)),
        port_(port),
        path_(std::move(path)),
        query_(std::move(query)),
        fragment_(std::move(fragment)) {
    setScheme(std::move(scheme), false);
    url_ = createUrl(scheme_, getHostAndPort(), path_, query_, fragment_);

    if (port_ == 0) {
      port_ = isSecure() ? 443 : 80;
    }
  }

  bool isValid() const noexcept {
    return valid_;
  }

  const std::string& getUrl() const noexcept {
    return url_;
  }

  uint16_t getPort() const noexcept {
    return port_;
  }

  const std::string& getScheme() const noexcept {
    return scheme_;
  }

  bool isSecure() const noexcept {
    return scheme_ == "https";
  }

  bool hasHost() const noexcept {
    return valid_ && !host_.empty();
  }

  const std::string& getHost() const noexcept {
    return host_;
  }

  std::string getHostAndPort() const noexcept {
    if (port_ == 0) {
      return host_;
    }
    std::stringstream ss;
    ss << host_ << ":" << port_;
    return ss.str();
  }

  std::string getHostAndPortOmitDefault() const noexcept {
    bool defaultPort = port_ == 0 || port_ == (isSecure() ? 443 : 80);
    if (defaultPort) {
      return host_;
    }
    std::stringstream ss;
    ss << host_ << ":" << port_;
    return ss.str();
  }

  const std::string& getPath() const noexcept {
    return path_;
  }

  const std::string& getQuery() const noexcept {
    return query_;
  }

  const std::string& getFragment() const noexcept {
    return fragment_;
  }

  std::string makeRelativeURL() const noexcept {
    std::stringstream ss;
    if (path_.empty()) {
      ss << "/";
    } else {
      ss << path_;
    }
    if (!query_.empty()) {
      ss << '?' << query_;
    }
    if (!fragment_.empty()) {
      ss << '#' << fragment_;
    }
    return ss.str();
  }

  friend bool operator==(const URL& lhs, const URL& rhs) {
    return lhs.getScheme() == rhs.getScheme() &&
           lhs.getHost() == rhs.getHost() && lhs.getPort() == rhs.getPort() &&
           lhs.getPath() == rhs.getPath() && lhs.getQuery() == rhs.getQuery() &&
           lhs.getFragment() == rhs.getFragment() &&
           lhs.getUrl() == rhs.getUrl();
  }

  friend bool operator!=(const URL& lhs, const URL& rhs) {
    return !(lhs == rhs);
  }

 private:
  void setScheme(std::string scheme, bool secure) {
    // empty scheme means it wasn't specified.  Caller can force it to secure
    if (scheme.empty() && secure) {
      scheme_ = "https";
    } else {
      scheme_ = std::move(scheme);
      folly::toLowerAscii(scheme_);
    }

    valid_ = (scheme_ == "http" || scheme_ == "https");
  }

  std::string scheme_;
  std::string host_;
  uint16_t port_;
  std::string path_;
  std::string query_;
  std::string fragment_;

  std::string url_;

  /* Does this represent a valid URL */
  bool valid_{false};
};

} // namespace proxygen
