/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/ParseURL.h>

#include <algorithm>
#include <charconv>

#include <fmt/format.h>
#include <folly/portability/Sockets.h>
#include <proxygen/lib/utils/UtilInl.h>

#include <proxygen/external/http_parser/http_parser.h>

namespace proxygen {

/**
 * According to RFC 3986, a generic HTTP URL is of the form:
 *   scheme:[//[user[:password]@]host[:port]][/path][?query][#fragment]
 *
 * ParseURL use http_parser to parse internet url, that supports internet
 * sematic url use double slash:
 *   http://host/path
 *   ftp://host/path
 *   rtmp://host/path
 *
 * It does not support special scheme like:
 *   mailto:user@host:port
 *   news:path
 *
 * And ParseURL support partial form (URI reference):
 *   host:port/path?query#fragment
 *   /path?query#fragment
 *   ?query
 *   #fragment
 *
 */

// Helper function to check if URL has valid scheme.
// http_parser only support full form scheme with double slash,
// and the scheme must be all alphabetic charecter.
static bool validateScheme(std::string_view url) {
  auto schemeEnd = url.find("://");
  if (schemeEnd == std::string_view::npos || schemeEnd == 0) {
    return false;
  }

  auto scheme = url.substr(0, schemeEnd);
  return std::all_of(scheme.begin(), scheme.end(), isAlpha);
}

bool ParseURL::isSupportedScheme(std::string_view location) {
  static constexpr std::array<std::string_view, 2> kSupportedSchemes{"http",
                                                                     "https"};
  auto schemeEnd = location.find("://");
  if (schemeEnd == std::string_view::npos) {
    // Location doesn't contain a scheme, so use the one from the original URL
    return true;
  }
  auto scheme = location.substr(0, schemeEnd);

  return std::find(kSupportedSchemes.begin(),
                   kSupportedSchemes.end(),
                   scheme) != kSupportedSchemes.end();
}

std::optional<std::string> ParseURL::getRedirectDestination(
    std::string_view url,
    std::string_view requestScheme,
    std::string_view location,
    std::string_view headerHost) noexcept {
  auto newUrl = ParseURL::parseURL(location);
  if (!newUrl) {
    DLOG(INFO) << "Unparsable location header=" << location;
    return std::nullopt;
  }
  if (!newUrl->hasHost()) {
    // New URL is relative
    std::optional<ParseURL> oldURL = ParseURL::parseURL(url);
    if (!oldURL || !oldURL->hasHost()) {
      // Old URL was relative, try host header
      oldURL = ParseURL::parseURL(headerHost);
      if (!oldURL || !oldURL->hasHost()) {
        VLOG(2) << "Cannot determine destination for relative redirect "
                << "location=" << location << " orig url=" << url
                << " host=" << headerHost;
        return std::nullopt;
      }
    } // else oldURL was absolute and has a host
    return fmt::format(
        "{}://{}{}", requestScheme, oldURL->hostAndPort(), location);
  } else {
    return std::string(newUrl->url());
  }
}

void ParseURL::parse(bool strict) noexcept {
  if (url_ == "/") {
    path_ = url_;
    valid_ = true;
    return;
  }
  if (validateScheme(url_)) {
    struct http_parser_url u {};
    memset(&u, 0, sizeof(struct http_parser_url)); // init before used
    valid_ = !(http_parser_parse_url_options(
        url_.data(),
        url_.size(),
        0,
        &u,
        strict ? F_PARSE_URL_OPTIONS_URL_STRICT : 0));

    if (valid_) {
      // Since we init the http_parser_url with all fields to 0, if the field
      // not present in url, it would be [0, 0], means that this field starts at
      // 0 and len = 0, we will get "" from this.  So no need to check field_set
      // before get field.

      scheme_ =
          url_.substr(u.field_data[UF_SCHEMA].off, u.field_data[UF_SCHEMA].len);

      if (u.field_data[UF_HOST].off != 0 &&
          url_[u.field_data[UF_HOST].off - 1] == '[') {
        // special case: host: [::1]
        host_ = url_.substr(u.field_data[UF_HOST].off - 1,
                            u.field_data[UF_HOST].len + 2);
      } else {
        host_ =
            url_.substr(u.field_data[UF_HOST].off, u.field_data[UF_HOST].len);
      }

      port_ = u.port;

      path_ = url_.substr(u.field_data[UF_PATH].off, u.field_data[UF_PATH].len);
      query_ =
          url_.substr(u.field_data[UF_QUERY].off, u.field_data[UF_QUERY].len);
      fragment_ = url_.substr(u.field_data[UF_FRAGMENT].off,
                              u.field_data[UF_FRAGMENT].len);

      authority_ = (port_) ? fmt::format("{}:{}", host_, port_) : host_;
    }
  } else {
    parseNonFully(strict);
  }
}

void ParseURL::parseNonFully(bool strict) noexcept {
  if (url_.empty()) {
    valid_ = false;
    return;
  }

  // Check if the URL has only printable characters and no control character.
  if (!validateURL(url_,
                   strict ? URLValidateMode::STRICT
                          : URLValidateMode::STRICT_COMPAT)) {
    valid_ = false;
    return;
  }

  auto pathStart = url_.find('/');
  auto queryStart = url_.find('?');
  auto hashStart = url_.find('#');

  auto queryEnd = std::min(hashStart, std::string_view::npos);
  auto pathEnd = std::min(queryStart, hashStart);
  auto authorityEnd = std::min(pathStart, pathEnd);

  authority_ = url_.substr(0, authorityEnd);

  if (pathStart < pathEnd) {
    path_ = url_.substr(pathStart, pathEnd - pathStart);
  } else {
    // missing the '/', e.g. '?query=3'
    path_ = "";
  }

  if (queryStart < queryEnd) {
    query_ = url_.substr(queryStart + 1, queryEnd - queryStart - 1);
  } else if (queryStart != std::string_view::npos && hashStart < queryStart) {
    valid_ = false;
    return;
  }

  if (hashStart != std::string_view::npos) {
    fragment_ = url_.substr(hashStart + 1);
  }

  if (!parseAuthority()) {
    valid_ = false;
    return;
  }

  valid_ = true;
}

bool ParseURL::parseAuthority() noexcept {
  constexpr auto npos = std::string_view::npos;
  std::string_view authority(authority_);
  auto left = authority.find('[');
  auto right = authority.find(']');

  auto pos = authority.find(':', right != npos ? right + 1 : 0);
  if (pos != npos) {
    auto port = authority.substr(pos + 1);
    auto end = port.data() + port.size();
    auto result = std::from_chars(port.data(), end, port_);
    if (result.ec != std::errc{} || result.ptr != end) {
      return false;
    }
  }

  if (left == npos && right == npos) {
    // not a ipv6 literal
    host_ = authority.substr(0, pos);
    return true;
  } else if (left < right && right != npos) {
    // a ipv6 literal
    host_ = authority.substr(left, right - left + 1);
    return true;
  } else {
    return false;
  }
}

bool ParseURL::hostIsIPAddress() {
  if (!valid_) {
    return false;
  }

  stripBrackets();
  // we have to make a copy of hostNoBrackets_ since the string_view is not
  // null-terminated
  std::string hostNoBrackets(hostNoBrackets_);
  int af = hostNoBrackets.find(':') == std::string::npos ? AF_INET : AF_INET6;
  char buf4[sizeof(in_addr)];
  char buf6[sizeof(in6_addr)];
  return inet_pton(af, hostNoBrackets.c_str(), af == AF_INET ? buf4 : buf6) ==
         1;
}

void ParseURL::stripBrackets() noexcept {
  if (hostNoBrackets_.empty()) {
    if (!host_.empty() && host_.front() == '[' && host_.back() == ']') {
      hostNoBrackets_ = host_.substr(1, host_.size() - 2);
    } else {
      hostNoBrackets_ = host_;
    }
  }
}

std::optional<std::string_view> ParseURL::getQueryParam(
    std::string_view query, const std::string_view name) noexcept {
  while (!query.empty()) {
    std::string_view param = query.substr(0, query.find('&'));
    query.remove_prefix(std::min(query.size(), param.size() + 1));
    if (!param.starts_with(name)) {
      continue;
    }
    param.remove_prefix(name.size());
    if (param.empty()) {
      return param;
    } else if (param.front() == '=') {
      param.remove_prefix(1);
      return param;
    }
  }
  return std::nullopt;
}

} // namespace proxygen
