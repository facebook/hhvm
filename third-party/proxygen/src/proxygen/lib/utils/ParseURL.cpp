/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/ParseURL.h>

#include <algorithm>
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
static bool validateScheme(folly::StringPiece url) {
  auto schemeEnd = url.find("://");
  if (schemeEnd == std::string::npos || schemeEnd == 0) {
    return false;
  }

  auto scheme = url.subpiece(0, schemeEnd);
  return std::all_of(
      scheme.begin(), scheme.end(), [](auto _) { return std::isalpha(_); });
}

void ParseURL::parse(bool strict) noexcept {
  if (url_.size() == 1 && url_[0] == '/') {
    path_ = url_;
    valid_ = true;
    return;
  }
  if (validateScheme(url_)) {
    struct http_parser_url u;
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

      scheme_ = url_.subpiece(u.field_data[UF_SCHEMA].off,
                              u.field_data[UF_SCHEMA].len);

      if (u.field_data[UF_HOST].off != 0 &&
          url_[u.field_data[UF_HOST].off - 1] == '[') {
        // special case: host: [::1]
        host_ = url_.subpiece(u.field_data[UF_HOST].off - 1,
                              u.field_data[UF_HOST].len + 2);
      } else {
        host_ =
            url_.subpiece(u.field_data[UF_HOST].off, u.field_data[UF_HOST].len);
      }

      port_ = u.port;

      path_ =
          url_.subpiece(u.field_data[UF_PATH].off, u.field_data[UF_PATH].len);
      query_ =
          url_.subpiece(u.field_data[UF_QUERY].off, u.field_data[UF_QUERY].len);
      fragment_ = url_.subpiece(u.field_data[UF_FRAGMENT].off,
                                u.field_data[UF_FRAGMENT].len);

      authority_ =
          (port_) ? folly::to<std::string>(host_, ":", port_) : host_.str();
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

  auto queryEnd = std::min(hashStart, std::string::npos);
  auto pathEnd = std::min(queryStart, hashStart);
  auto authorityEnd = std::min(pathStart, pathEnd);

  authority_ = url_.subpiece(0, authorityEnd).str();

  if (pathStart < pathEnd) {
    path_ = url_.subpiece(pathStart, pathEnd - pathStart);
  } else {
    // missing the '/', e.g. '?query=3'
    path_ = "";
  }

  if (queryStart < queryEnd) {
    query_ = url_.subpiece(queryStart + 1, queryEnd - queryStart - 1);
  } else if (queryStart != std::string::npos && hashStart < queryStart) {
    valid_ = false;
    return;
  }

  if (hashStart != std::string::npos) {
    fragment_ = url_.subpiece(hashStart + 1, std::string::npos);
  }

  if (!parseAuthority()) {
    valid_ = false;
    return;
  }

  valid_ = true;
}

bool ParseURL::parseAuthority() noexcept {
  auto left = authority_.find("[");
  auto right = authority_.find("]");

  auto pos = authority_.find(":", right != std::string::npos ? right : 0);
  if (pos != std::string::npos) {
    try {
      port_ = folly::to<uint16_t>(
          folly::StringPiece(authority_, pos + 1, std::string::npos));
    } catch (...) {
      return false;
    }
  }

  if (left == std::string::npos && right == std::string::npos) {
    // not a ipv6 literal
    host_ = folly::StringPiece(authority_, 0, pos);
    return true;
  } else if (left < right && right != std::string::npos) {
    // a ipv6 literal
    host_ = folly::StringPiece(authority_, left, right - left + 1);
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
  int af = hostNoBrackets_.find(':') == std::string::npos ? AF_INET : AF_INET6;
  char buf4[sizeof(in_addr)];
  char buf6[sizeof(in6_addr)];
  // we have to make a copy of hostNoBrackets_ since the string piece is not
  // null-terminated
  return inet_pton(af,
                   hostNoBrackets_.str().c_str(),
                   af == AF_INET ? buf4 : buf6) == 1;
}

void ParseURL::stripBrackets() noexcept {
  if (hostNoBrackets_.empty()) {
    if (!host_.empty() && host_.front() == '[' && host_.back() == ']') {
      hostNoBrackets_ = host_.subpiece(1, host_.size() - 2);
    } else {
      hostNoBrackets_ = host_;
    }
  }
}

folly::Optional<folly::StringPiece> ParseURL::getQueryParam(
    folly::StringPiece name) const noexcept {
  auto params = query_;
  while (!params.empty()) {
    auto param = params.split_step('&');
    if (!param.removePrefix(name)) {
      continue;
    }
    if (!param.removePrefix('=') && !param.empty()) {
      continue;
    }
    return param;
  }
  return {};
}

} // namespace proxygen
