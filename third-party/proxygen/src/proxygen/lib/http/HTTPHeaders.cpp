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

namespace {

/**
 * The initial capacity of the three vectors, reserved right after
 * construction.
 */
static constexpr size_t kInitialVectorReserve = 16;
static constexpr size_t kRecSize =
    (sizeof(char) + sizeof(std::string*) + sizeof(std::string));

// iterate over the positions (in vector) of all headers with given code
#define ITERATE_OVER_CODES(Code, Block)                   \
  {                                                       \
    const HTTPHeaderCode* ptr = codes();                  \
    while (ptr) {                                         \
      ptr = (HTTPHeaderCode*)memchr(                      \
          (void*)ptr, (Code), length_ - (ptr - codes())); \
      if (ptr == nullptr)                                 \
        break;                                            \
      const size_t pos = ptr - codes();                   \
      {Block} ptr++;                                      \
    }                                                     \
  }                                                       \
  static_assert(true, "semicolon required")

// iterate over the positions of all headers with given name
#define ITERATE_OVER_STRINGS(String, Block)               \
  ITERATE_OVER_CODES(HTTPHeaderCode::HTTP_HEADER_OTHER, { \
    if (caseInsensitiveEqual((String), *names()[pos])) {  \
      {                                                   \
        Block                                             \
      }                                                   \
    }                                                     \
  })

// iterate over the positions of all headers with given name ignoring - and _
#define ITERATE_OVER_STRINGS_ALL_VERSION(String, Block)            \
  ITERATE_OVER_CODES(HTTP_HEADER_OTHER, {                          \
    if (caseUnderscoreInsensitiveEqual((String), *names()[pos])) { \
      {                                                            \
        Block                                                      \
      }                                                            \
    }                                                              \
  })

} // namespace

namespace proxygen {

const string empty_string;

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

void HTTPHeaders::add(folly::StringPiece name, std::string&& value) {
  assert(name.size());
  const HTTPHeaderCode code = HTTPCommonHeaders::hash(name.data(), name.size());
  auto namePtr =
      ((code == HTTPHeaderCode::HTTP_HEADER_OTHER)
           ? new std::string(name.data(), name.size())
           : (std::string*)HTTPCommonHeaders::getPointerToName(code));
  emplace_back(code, namePtr, std::move(value));
}

void HTTPHeaders::add(HTTPHeaderCode code, std::string&& value) {
  auto namePtr = (std::string*)HTTPCommonHeaders::getPointerToName(code);
  emplace_back(code, namePtr, std::move(value));
}

void HTTPHeaders::add(HTTPHeaders::headers_initializer_list l) {
  for (auto& p : l) {
    p.first.type_ == HTTPHeaderName::CODE ? add(p.first.code_, p.second)
                                          : add(p.first.name_, p.second);
  }
}

bool HTTPHeaders::exists(folly::StringPiece name) const {
  const HTTPHeaderCode code = HTTPCommonHeaders::hash(name.data(), name.size());
  if (code != HTTP_HEADER_OTHER) {
    return exists(code);
  }
  ITERATE_OVER_STRINGS(name, { return true; });
  return false;
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
  }
  bool removed = false;
  ITERATE_OVER_STRINGS(name, {
    delete names()[pos];
    codes()[pos] = HTTP_HEADER_NONE;
    removed = true;
    ++deletedCount_;
  });
  return removed;
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
      HTTP_HEADER_CONNECTION, [&](const std::string& value) -> bool {
        // Remove all headers specified in Connection header
        // look for multiple values separated by commas
        char const* str = value.c_str();

        // skip leading whitespace
        while (isLWS(*str)) {
          str++;
        }

        while (*str != 0) {
          char const* pos = strchr(str, ',');
          if (pos == nullptr) {
            // last (or only) token, done

            // count chars in the token
            len = 0;
            while (str[len] != 0 && !isLWS(str[len])) {
              len++;
            }
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
          while (len > 0 && isLWS(str[len - 1])) {
            len--;
          }
          if (len > 0) {
            // non-empty token
            string hdr(str, len);
            if (transferHeaderIfPresent(hdr, strippedHeaders)) {
              VLOG(3) << "Stripped connection-named hop-by-hop header " << hdr;
            }
          } // else empty token, no-op
          str = pos + 1;

          // skip whitespace
          while (isLWS(*str)) {
            str++;
          }
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
                        std::string(values()[i]));
    }
  }
}

HTTPHeaders::SingleOrNullptrResult HTTPHeaders::getSingleOrNullptr(
    HTTPHeaderCode code) const noexcept {
  SingleOrNullptrResult res;
  forEachValueOfHeader(code, [&](const std::string& value) -> bool {
    res.value = (res.value == nullptr) ? &value : nullptr;
    res.exists = true;
    return res.value == nullptr; // stop if seen before
  });
  return res;
}

HTTPHeaders::SingleOrNullptrResult HTTPHeaders::getSingleOrNullptr(
    folly::StringPiece name) const noexcept {
  SingleOrNullptrResult res;
  forEachValueOfHeader(name, [&](const std::string& value) -> bool {
    res.value = (res.value == nullptr) ? &value : nullptr;
    res.exists = true;
    return res.value == nullptr; // stop if seen before
  });
  return res;
}

const std::string& HTTPHeaders::getSingleOrEmpty(HTTPHeaderCode code) const {
  return *getSingleOrNullptr(code);
}
const std::string& HTTPHeaders::getSingleOrEmpty(
    folly::StringPiece name) const {
  return *getSingleOrNullptr(name);
}

HTTPHeaderCode* HTTPHeaders::codes() const noexcept {
  return codes(memory_.get(), capacity_);
}

HTTPHeaderCode* HTTPHeaders::codes(const uint8_t* memory,
                                   size_t capacity) const noexcept {
  return (HTTPHeaderCode*)(memory + capacity * (sizeof(std::string*) +
                                                sizeof(std::string)));
}

std::string** HTTPHeaders::names() const noexcept {
  return names(memory_.get(), capacity_);
}

std::string** HTTPHeaders::names(const uint8_t* memory,
                                 size_t capacity) const noexcept {
  return (std::string**)(memory + capacity * sizeof(std::string));
}

std::string* HTTPHeaders::values() const noexcept {
  return values(memory_.get(), capacity_);
}

std::string* HTTPHeaders::values(const uint8_t* memory, size_t) const noexcept {
  return (std::string*)(memory);
}

void HTTPHeaders::ensure(size_t minCapacity) {
  if (capacity_ >= minCapacity) {
    return;
  }

  static_assert(kInitialVectorReserve >= 1,
                "This loop depends on a strictly-positive "
                "kInitialVectorReserve to terminate");
  size_t targetCapacity = std::max(capacity_, kInitialVectorReserve);
  while (targetCapacity < minCapacity) {
    // targetCapacity will never be zero, so it will always grow here.
    targetCapacity += targetCapacity / 2;
  }
  resize(targetCapacity);
}

void HTTPHeaders::resize(size_t capacity) {
  if (capacity <= capacity_) {
    return;
  }
  auto newMemory = std::make_unique<uint8_t[]>(capacity * kRecSize);
  if (length_ > 0) {
    memcpy(codes(newMemory.get(), capacity), codes(), length_);
    memcpy(names(newMemory.get(), capacity),
           names(),
           sizeof(std::string*) * length_);
    auto vNew = values(newMemory.get(), capacity);
    auto v = values();
    for (size_t i = 0; i < length_; i++) {
      new (vNew + i) std::string(std::move(v[i]));
    }
  }
  memory_ = std::move(newMemory);
  capacity_ = capacity;
}

void HTTPHeaders::emplace_back(HTTPHeaderCode code,
                               std::string* name,
                               std::string&& value) {
  ensure(length_ + 1);
  codes()[length_] = code;
  names()[length_] = name;
  std::string* p = values() + length_++;
  auto trimmed = folly::trimWhitespace(value);
  if (LIKELY(trimmed.size() == value.size())) { // elide copy
    new (p) std::string(std::move(value));
  } else {
    new (p) std::string(trimmed);
  }
}

void HTTPHeaders::forEach(const ForEachFnT& func) const {
  auto c = codes();
  auto n = names();
  auto v = values();
  for (size_t i = 0; i < length_; ++i) {
    if (c[i] != HTTPHeaderCode::HTTP_HEADER_NONE) {
      func(*n[i], v[i]);
    }
  }
}

void HTTPHeaders::forEachWithCode(const ForEachWithCodeFnT& func) const {
  auto c = codes();
  auto n = names();
  auto v = values();
  for (size_t i = 0; i < length_; ++i) {
    if (c[i] != HTTPHeaderCode::HTTP_HEADER_NONE) {
      func(c[i], *n[i], v[i]);
    }
  }
}

bool HTTPHeaders::forEachValueOfHeader(
    folly::StringPiece name, const ForEachValueOfHeaderFnT& func) const {
  const HTTPHeaderCode code = HTTPCommonHeaders::hash(name.data(), name.size());
  if (code != HTTPHeaderCode::HTTP_HEADER_OTHER) {
    return forEachValueOfHeader(code, func);
  }
  ITERATE_OVER_STRINGS(name, {
    if (func(values()[pos])) {
      return true;
    }
  });
  return false;
}

bool HTTPHeaders::forEachValueOfHeader(
    HTTPHeaderCode code, const ForEachValueOfHeaderFnT& func) const {
  ITERATE_OVER_CODES(code, {
    if (func(values()[pos])) {
      return true;
    }
  });
  return false;
}

bool HTTPHeaders::removeByPredicate(const RemoveByPredFnT& func) {
  bool removed = false;
  auto c = codes();
  auto n = names();
  auto v = values();
  for (size_t i = 0; i < length_; ++i) {
    if (c[i] == HTTPHeaderCode::HTTP_HEADER_NONE || !func(c[i], *n[i], v[i])) {
      continue;
    }

    if (c[i] == HTTPHeaderCode::HTTP_HEADER_OTHER) {
      delete n[i];
      n[i] = nullptr;
    }

    c[i] = HTTPHeaderCode::HTTP_HEADER_NONE;
    ++deletedCount_;
    removed = true;
  }

  return removed;
}

} // namespace proxygen
