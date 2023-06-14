/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define PROXYGEN_HTTPHEADERS_IMPL
#include <proxygen/lib/http/HTTPHeaders.h>

#include <glog/logging.h>

using std::bitset;
using std::string;

namespace proxygen {

const string empty_string;
const std::string HTTPHeaders::COMBINE_SEPARATOR = ", ";

bitset<256>& HTTPHeaders::perHopHeaderCodes() {
  static bitset<256> perHopHeaderCodes{[] {
    bitset<256> bs;
    bs[HTTP_HEADER_CONNECTION] = true;
    bs[HTTP_HEADER_KEEP_ALIVE] = true;
    bs[HTTP_HEADER_PROXY_AUTHENTICATE] = true;
    bs[HTTP_HEADER_PROXY_AUTHORIZATION] = true;
    bs[HTTP_HEADER_PROXY_CONNECTION] = true;
    bs[HTTP_HEADER_TE] = true;
    bs[HTTP_HEADER_TRAILER] = true;
    bs[HTTP_HEADER_TRANSFER_ENCODING] = true;
    bs[HTTP_HEADER_UPGRADE] = true;
    return bs;
  }()};
  return perHopHeaderCodes;
}

HTTPHeaders::HTTPHeaders() : deletedCount_(0) {
  resize(kInitialVectorReserve);
}

void HTTPHeaders::add(folly::StringPiece name, folly::StringPiece value) {
  CHECK(name.size());
  const HTTPHeaderCode code = HTTPCommonHeaders::hash(name.data(), name.size());
  emplace_back(code,
               ((code == HTTP_HEADER_OTHER)
                    ? new std::string(name.data(), name.size())
                    : (std::string*)HTTPCommonHeaders::getPointerToName(code)),
               value);
}

void HTTPHeaders::add(HTTPHeaders::headers_initializer_list l) {
  for (auto& p : l) {
    if (p.first.type_ == HTTPHeaderName::CODE) {
      add(p.first.code_, folly::StringPiece(p.second.data(), p.second.size()));
    } else {
      add(p.first.name_, folly::StringPiece(p.second.data(), p.second.size()));
    }
  }
}

void HTTPHeaders::rawAdd(const std::string& name, const std::string& value) {
  add(name, value);
}

void HTTPHeaders::addFromCodec(const char* str, size_t len, string&& value) {
  const HTTPHeaderCode code = HTTPCommonHeaders::hash(str, len);
  auto namePtr = (code == HTTP_HEADER_OTHER)
                     ? new string(str, len)
                     : (std::string*)HTTPCommonHeaders::getPointerToName(code);

  emplace_back(code, namePtr, std::move(value));
}

bool HTTPHeaders::exists(folly::StringPiece name) const {
  const HTTPHeaderCode code = HTTPCommonHeaders::hash(name.data(), name.size());
  if (code != HTTP_HEADER_OTHER) {
    return exists(code);
  } else {
    ITERATE_OVER_STRINGS(name, { return true; });
    return false;
  }
}

bool HTTPHeaders::exists(HTTPHeaderCode code) const {
  return length_ > 0 && memchr((void*)codes(), code, length_) != nullptr;
}

size_t HTTPHeaders::getNumberOfValues(HTTPHeaderCode code) const {
  size_t count = 0;
  ITERATE_OVER_CODES(code, {
    (void)pos;
    ++count;
  });
  return count;
}

size_t HTTPHeaders::getNumberOfValues(folly::StringPiece name) const {
  size_t count = 0;
  forEachValueOfHeader(name, [&](folly::StringPiece /*value*/) -> bool {
    ++count;
    return false;
  });
  return count;
}

bool HTTPHeaders::remove(folly::StringPiece name) {
  const HTTPHeaderCode code = HTTPCommonHeaders::hash(name.data(), name.size());
  if (code != HTTP_HEADER_OTHER) {
    return remove(code);
  } else {
    bool removed = false;
    ITERATE_OVER_STRINGS(name, {
      delete names()[pos];
      codes()[pos] = HTTP_HEADER_NONE;
      removed = true;
      ++deletedCount_;
    });
    return removed;
  }
}

bool HTTPHeaders::remove(HTTPHeaderCode code) {
  bool removed = false;
  ITERATE_OVER_CODES(code, {
    codes()[pos] = HTTP_HEADER_NONE;
    removed = true;
    ++deletedCount_;
  });
  return removed;
}

bool HTTPHeaders::removeAllVersions(HTTPHeaderCode code,
                                    folly::StringPiece name) {
  bool removed = false;
  if (code != HTTP_HEADER_OTHER) {
    removed = remove(code);
  }
  ITERATE_OVER_STRINGS_ALL_VERSION(name, {
    delete names()[pos];
    codes()[pos] = HTTP_HEADER_NONE;
    removed = true;
    ++deletedCount_;
  });
  return removed;
}

void HTTPHeaders::disposeOfHeaderNames() {
  ITERATE_OVER_CODES(HTTP_HEADER_OTHER, { delete names()[pos]; });
}

void HTTPHeaders::destroy() {
  auto c = codes();
  auto n = names();
  auto v = values();
  for (size_t i = 0; i < length_; ++i) {
    if (c[i] == HTTP_HEADER_OTHER) {
      delete n[i];
    }
    auto p = v + i;
    p->~string();
  }
}

HTTPHeaders::~HTTPHeaders() {
  destroy();
}

HTTPHeaders::HTTPHeaders(const HTTPHeaders& hdrs)
    : length_(0), capacity_(0), deletedCount_(hdrs.deletedCount_) {
  copyFrom(hdrs);
}

HTTPHeaders::HTTPHeaders(HTTPHeaders&& hdrs) noexcept
    : memory_(std::move(hdrs.memory_)),
      length_(hdrs.length_),
      capacity_(hdrs.capacity_),
      deletedCount_(hdrs.deletedCount_) {
  hdrs.length_ = 0;
  hdrs.capacity_ = 0;
  hdrs.deletedCount_ = 0;
}

void HTTPHeaders::copyFrom(const HTTPHeaders& other) {
  ensure(other.capacity_);
  memcpy(codes(), other.codes(), other.length_);
  for (size_t i = 0; i < other.length_; i++) {
    if (codes()[i] == HTTP_HEADER_OTHER) {
      names()[i] = new std::string(*other.names()[i]);
    } else {
      names()[i] = other.names()[i];
    }
    new (values() + i) std::string(other.values()[i]);
  }
  length_ = other.length_;
}

HTTPHeaders& HTTPHeaders::operator=(const HTTPHeaders& hdrs) {
  if (this != &hdrs) {
    removeAll();
    copyFrom(hdrs);
  }
  return *this;
}

HTTPHeaders& HTTPHeaders::operator=(HTTPHeaders&& hdrs) {
  if (this != &hdrs) {
    removeAll();
    std::swap(memory_, hdrs.memory_);
    std::swap(capacity_, hdrs.capacity_);
    length_ = hdrs.length_;
    hdrs.length_ = 0;
    deletedCount_ = hdrs.deletedCount_;
    hdrs.deletedCount_ = 0;
  }

  return *this;
}

void HTTPHeaders::removeAll() {
  destroy();
  length_ = 0;
  deletedCount_ = 0;
}

size_t HTTPHeaders::size() const {
  return length_ - deletedCount_;
}

bool HTTPHeaders::transferHeaderIfPresent(folly::StringPiece name,
                                          HTTPHeaders& strippedHeaders) {
  bool transferred = false;
  const HTTPHeaderCode code = HTTPCommonHeaders::hash(name.data(), name.size());
  if (code == HTTP_HEADER_OTHER) {
    ITERATE_OVER_STRINGS(name, {
      strippedHeaders.emplace_back(
          HTTP_HEADER_OTHER, names()[pos], std::move(values()[pos]));
      codes()[pos] = HTTP_HEADER_NONE;
      transferred = true;
      ++deletedCount_;
    });
  } else { // code != HTTP_HEADER_OTHER
    ITERATE_OVER_CODES(code, {
      strippedHeaders.emplace_back(
          code, names()[pos], std::move(values()[pos]));
      codes()[pos] = HTTP_HEADER_NONE;
      transferred = true;
      ++deletedCount_;
    });
  }
  return transferred;
}

void HTTPHeaders::stripPerHopHeaders(HTTPHeaders& strippedHeaders,
                                     bool stripPriority,
                                     const HTTPHeaders* customPerHopHeaders) {
  int len;
  forEachValueOfHeader(
      HTTP_HEADER_CONNECTION, [&](const string& stdStr) -> bool {
        // Remove all headers specified in Connection header
        // look for multiple values separated by commas
        char const* str = stdStr.c_str();

        // skip leading whitespace
        while (isLWS(*str))
          str++;

        while (*str != 0) {
          char const* pos = strchr(str, ',');
          if (pos == nullptr) {
            // last (or only) token, done

            // count chars in the token
            len = 0;
            while (str[len] != 0 && !isLWS(str[len]))
              len++;
            if (len > 0) {
              string hdr(str, len);
              if (transferHeaderIfPresent(hdr, strippedHeaders)) {
                VLOG(3) << "Stripped connection-named hop-by-hop header "
                        << hdr;
              }
            }
            break;
          }
          len = pos - str;
          // strip trailing whitespace
          while (len > 0 && isLWS(str[len - 1]))
            len--;
          if (len > 0) {
            // non-empty token
            string hdr(str, len);
            if (transferHeaderIfPresent(hdr, strippedHeaders)) {
              VLOG(3) << "Stripped connection-named hop-by-hop header " << hdr;
            }
          } // else empty token, no-op
          str = pos + 1;

          // skip whitespace
          while (isLWS(*str))
            str++;
        }
        return false; // continue processing "connection" headers
      });

  // Strip hop-by-hop headers
  auto& perHopHeaders = perHopHeaderCodes();
  for (size_t i = 0; i < length_; ++i) {
    auto& code = codes()[i];
    bool perHop = false;
    if (code != HTTP_HEADER_OTHER) {
      perHop = (perHopHeaders[code] ||
                (stripPriority && code == HTTP_HEADER_PRIORITY) ||
                (customPerHopHeaders && customPerHopHeaders->exists(code)));
    } else if (customPerHopHeaders &&
               customPerHopHeaders->exists(*names()[i])) {
      perHop = true;
    }
    if (perHop) {
      strippedHeaders.emplace_back(code, names()[i], std::move(values()[i]));
      code = HTTP_HEADER_NONE;
      ++deletedCount_;
      VLOG(5) << "Stripped hop-by-hop header " << *names()[i];
    }
  }
}

void HTTPHeaders::copyTo(HTTPHeaders& hdrs) const {
  hdrs.ensure(hdrs.size() + size());
  for (size_t i = 0; i < length_; ++i) {
    if (codes()[i] != HTTP_HEADER_NONE) {
      hdrs.emplace_back(codes()[i],
                        ((codes()[i] == HTTP_HEADER_OTHER)
                             ? new string(*names()[i])
                             : names()[i]),
                        values()[i]);
    }
  }
}

} // namespace proxygen
