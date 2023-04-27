/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <assert.h>
#include <cctype>
#include <folly/Range.h>
#include <folly/portability/Windows.h> // for windows compatibility: STRICT maybe defined by some win headers
#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/codec/compress/Header.h>
#include <proxygen/lib/utils/UtilInl.h>
#include <stdint.h>
#include <string>

namespace proxygen {

/**
 * On some mobile platforms we don't always get a chance to stop proxygen
 * eventbase. That leads to static object being cleaned up by system while
 * proxygen tries to access them, which is bad. The speedup from making
 * some variable static isn't necessary on mobile clients anyway.
 */
#ifdef FOLLY_MOBILE
#define CODEC_STATIC
#else
#define CODEC_STATIC static
#endif

folly::Optional<HTTPPriority> parseHTTPPriorityString(
    folly::StringPiece priorityString);

class CodecUtil {
 public:
  // If these are needed elsewhere, we can move them to a more generic
  // namespace/class later
  static const char http_tokens[256];

  static bool validateURL(folly::ByteRange url, URLValidateMode mode) {
    return proxygen::validateURL(url, mode);
  }

  static bool isalpha(uint8_t c) {
    return ((unsigned int)(c | 32) - 97) < 26U;
  }

  static bool validateMethod(folly::ByteRange method) {
    for (auto p = method.begin(); p != method.end(); p++) {
      // '-' is valid except for start and end
      if (*p == '-' && p != method.begin() && p != method.end()) {
        continue;
      }
      if (!CodecUtil::isalpha(*p)) {
        return false;
      }
    }
    return true;
  }

  static bool validateScheme(folly::ByteRange method) {
    for (auto p : method) {
      if (!CodecUtil::isalpha(p)) {
        // methods are all characters
        return false;
      }
    }
    return true;
  }

  enum HeaderNameValidationMode {
    HEADER_NAME_STRICT_COMPAT,
    HEADER_NAME_STRICT
  };
  static bool validateHeaderName(folly::ByteRange name,
                                 HeaderNameValidationMode mode) {
    if (name.size() == 0) {
      return false;
    }
    for (uint8_t p : name) {
      if (mode == HEADER_NAME_STRICT_COMPAT) {
        // Allows ' ', '"', '/', '}' and high ASCII
        if (p < 0x80 && !http_tokens[p]) {
          return false;
        }
      } else {
        if ((p < 0x80 && (http_tokens[p] != p)) || p >= 0x80) {
          return false;
        }
      }
    }
    return true;
  }

  /**
   * RFC2616 allows certain control chars in header values if they are
   * quoted and escaped.
   * When mode is COMPLIANT, then this is allowed.
   * When mode is STRICT*, no escaped CTLs are allowed
   *
   * An unfortunate side effect when this function moved from signed to unsigned
   * chars, the high-ASCII check was broken.  Temporarily continue to allow this
   * with a special mode.
   */
  enum CtlEscapeMode { COMPLIANT, STRICT_COMPAT, STRICT };

  static bool validateHeaderValue(folly::ByteRange value, CtlEscapeMode mode) {
    bool escape = false;
    bool quote = false;
    enum {
      lws_none,
      lws_expect_nl,
      lws_expect_ws1,
      lws_expect_ws2
    } state = lws_none;

    for (auto p = std::begin(value); p != std::end(value); ++p) {
      if (escape) {
        escape = false;
        if (mode == COMPLIANT) {
          // prev char escaped.  Turn off escape and go to next char
          // COMPLIANT mode only
          assert(quote);
          continue;
        }
      }
      switch (state) {
        case lws_none:
          switch (*p) {
            case '\\':
              if (quote) {
                escape = true;
              }
              break;
            case '\"':
              quote = !quote;
              break;
            case '\r':
              state = lws_expect_nl;
              break;
            default:
              if ((*p < 0x20 && *p != '\t') || (*p == 0x7f) ||
                  (*p > 0x7f && mode == STRICT)) {
                // unexpected ctl per rfc2616, HT OK
                return false;
              }
              break;
          }
          break;
        case lws_expect_nl:
          if (*p != '\n') {
            // unescaped \r must be LWS
            return false;
          }
          state = lws_expect_ws1;
          break;
        case lws_expect_ws1:
          if (*p != ' ' && *p != '\t') {
            // unescaped \r\n must be LWS
            return false;
          }
          state = lws_expect_ws2;
          break;
        case lws_expect_ws2:
          if (*p != ' ' && *p != '\t') {
            // terminated LWS
            state = lws_none;
            // check this char again
            p--;
          }
          break;
      }
    }
    // Unterminated quotes are OK, since the value can be* TEXT which treats
    // the " like any other char.
    // Unterminated escapes are bad because it will escape the next character
    // when converting to HTTP
    // Unterminated LWS (dangling \r or \r\n) is bad because it could
    // prematurely terminate the headers when converting to HTTP
    return !escape && (state == lws_none || state == lws_expect_ws2);
  }

  static bool hasGzipAndDeflate(const std::string& value,
                                bool& hasGzip,
                                bool& hasDeflate);

  static bool appendHeaders(const HTTPHeaders& inputHeaders,
                            std::vector<compress::Header>& headers,
                            HTTPHeaderCode headerToCheck);

  static const std::bitset<256>& perHopHeaderCodes();
};

} // namespace proxygen
