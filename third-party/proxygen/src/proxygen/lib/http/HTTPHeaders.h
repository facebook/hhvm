/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/FBVector.h>
#include <folly/Range.h>
#include <folly/String.h>
#include <folly/Utility.h>
#include <proxygen/lib/http/HTTPCommonHeaders.h>
#include <proxygen/lib/utils/Export.h>
#include <proxygen/lib/utils/UtilInl.h>

#include <bitset>
#include <cstring>
#include <initializer_list>
#include <string>
#include <string_view>

namespace proxygen {

extern const std::string empty_string;

/**
 * Return true if the character is linear whitespace, as defined by the LWS
 * definition in RFC 2616, and false otherwise
 */
inline bool isLWS(char c) {
  // Technically \r and \n are only allowed in LWS if they appear together.
  if (c == ' ' || c == '\n' || c == '\t' || c == '\r') {
    return true;
  }
  return false;
}

/**
 * A collection of HTTP headers.
 *
 * This is broken out from HTTPMessage, as it's convenient for other things to
 * be able to use collections of HTTP headers that are easy to work with. The
 * structure is optimized for real-life header collection sizes.
 *
 * Headers are stored as Name/Value pairs, in the order they are received on
 * the wire. We hash the names of all common HTTP headers (using a static
 * perfect hash function generated using gperf from HTTPCommonHeaders.gperf)
 * into 1-byte hashes (we call them "codes") and only store these. We search
 * them using memchr, which has an x86_64 assembly implementation with
 * complexity O(n/16) ;)
 *
 * Instead of creating strings with header names, we point to a static array
 * of strings in HTTPCommonHeaders. If the header name is not in our set of
 * common header names (this is considered unlikely, because we intend this set
 * to be very complete), then we create a new string with its name (we own that
 * pointer then). For such headers, we store the code HTTP_HEADER_OTHER.
 *
 * The code HTTP_HEADER_NONE signifies a header that has been removed.
 *
 * Most methods which take a header name have two versions: one accepting
 * a string, and one accepting a code. It is recommended to use the latter
 * if possible, as in:
 *     headers.add(HTTP_HEADER_LOCATION, location);
 * rather than:
 *     headers.add("Location", location);
 */
class HTTPHeaders {
 public:
  struct HTTPHeaderName {
    enum Type { CODE, STRING };
    union {
      folly::StringPiece name_;
      HTTPHeaderCode code_;
    };
    Type type_;
    /* implicit */ HTTPHeaderName(HTTPHeaderCode code)
        : code_(code), type_(CODE) {
    }
    /* implicit */ HTTPHeaderName(const char* name)
        : name_(name), type_(STRING) {
    }
    /* implicit */ HTTPHeaderName(folly::StringPiece name)
        : name_(name), type_(STRING) {
    }
  };

  using headers_initializer_list =
      std::initializer_list<std::pair<HTTPHeaderName, folly::StringPiece>>;

  /*
   * separator used to concatenate multiple values of the same header
   * check out sections 4.2 and 14.45 from rfc2616
   */
  static constexpr std::string_view kCombineSeparator{", "};

  FB_EXPORT HTTPHeaders();
  FB_EXPORT ~HTTPHeaders();
  FB_EXPORT HTTPHeaders(const HTTPHeaders&);
  FB_EXPORT HTTPHeaders& operator=(const HTTPHeaders&);
  FB_EXPORT HTTPHeaders(HTTPHeaders&&) noexcept;
  FB_EXPORT HTTPHeaders& operator=(HTTPHeaders&&);

  /**
   * Add the header 'name' with value 'value'; if other instances of this
   * header name exist, they will be retained.
   */
  void add(folly::StringPiece name, std::string&& value);
  void add(folly::StringPiece name, const std::string& value) {
    add(name, std::string(value));
  }
  void add(folly::StringPiece name, folly::StringPiece value) {
    add(name, std::string(value));
  }
  // TODO(@damlaj): remove this fn & patch up all callsites
  void add(folly::StringPiece name, const char* value) {
    return add(name, std::string(value));
  }

  void add(HTTPHeaderCode code, std::string&& value);
  void add(HTTPHeaderCode code, const std::string& value) {
    add(code, std::string(value));
  }
  void add(HTTPHeaderCode code, folly::StringPiece value) {
    add(code, std::string(value));
  }
  // TODO(@damlaj): remove this fn & patch up all callsites
  void add(HTTPHeaderCode code, const char* value) {
    return add(code, std::string(value));
  }

  void add(headers_initializer_list l);

  /**
   * For the header 'name', set its value to the single header 'value',
   * removing any other instances of this header.
   */
  void set(folly::StringPiece name, const std::string& value) {
    // this could be somewhat optimized but probably not an issue yet
    remove(name);
    add(name, value);
  }
  void set(HTTPHeaderCode code, const std::string& value) {
    remove(code);
    add(code, value);
  }

  /**
   * This method will set only one version of the header, it will first
   * Remove all possible versions of header eg. if x-y-z is the
   * argument it will remove x-y_z, x_y-z and x_y_z too and then set the given
   * header name, value.
   */
  void setOneVersion(folly::StringPiece name,
                     HTTPHeaderCode code,
                     const std::string& value) {
    removeAllVersions(code, name);
    add(name, value);
  }

  /**
   * Do we have an instance of the given header?
   */
  [[nodiscard]] bool exists(folly::StringPiece name) const;
  [[nodiscard]] bool exists(HTTPHeaderCode code) const;
  bool rawExists(std::string& name) const {
    return exists(name);
  }

  /**
   * combine all the value for this header into a string
   */
  template <typename T>
  std::string combine(
      const T& header,
      const std::string_view separator = kCombineSeparator) const;

  /**
   * Process the list of all headers, in the order that they were seen:
   * for each header:value pair, the function/functor/lambda-expression
   * given as the second parameter will be executed. It should take two
   * const string & parameters and return void. Example use:
   *     hdrs.forEach([&] (const string& header, const std::string& val) {
   *       std::cout << header << ": " << val;
   *     });
   */
  using ForEachFnT =
      std::function<void(const std::string&, const std::string&)>;
  void forEach(const ForEachFnT& func) const;

  /**
   * Process the list of all headers, in the order that they were seen:
   * for each header:value pair, the function/functor/lambda-expression
   * given as the second parameter will be executed. It should take one
   * HTTPHeaderCode (code) parameter, two const string & parameters and
   * return void. Example use:
   *     hdrs.forEachWithCode([&] (HTTPHeaderCode code,
   *                               const string& header,
   *                               const std::string& val) {
   *       std::cout << header << "(" << code << "): " << val;
   *     });
   */
  using ForEachWithCodeFnT = std::function<void(
      HTTPHeaderCode, const std::string&, const std::string&)>;
  void forEachWithCode(const ForEachWithCodeFnT& func) const;

  /**
   * Process the list of all headers, in the order that they were seen:
   * for each header:value pair, the function/functor/lambda-expression
   * given as the parameter will be executed to determine whether the
   * header should be removed. Example use:
   *
   *     hdrs.removeByPredicate([&] (HTTPHeaderCode code,
   *                                 const string& header,
   *                                 const string& val) {
   *       return std::regex_match(header, std::regex("^X-Fb-.*"));
   *     });
   *
   * return true only if one or more headers are removed.
   */

  using RemoveByPredFnT = std::function<bool(
      HTTPHeaderCode, const std::string&, const std::string&)>;
  bool removeByPredicate(const RemoveByPredFnT& func);

  /**
   * Returns the value of the header if it's found in the message and is the
   * only value under the given name. If either of these is violated, returns
   * nullptr. `exists` member can help distinguish whether nullptr was returned
   * due to multiple ocurrences or not found.
   *
   * For code that follows `HTTPHeaders::exists ->
   * HTTPHeaders::getSingleOrEmpty` pattern, it can be replaced with
   * `HTTPHeader::getSingleOrNullptr` to elide a memchr lookup in the happy path
   */
  struct SingleOrNullptrResult {
    const std::string* value{nullptr};
    bool exists{false};

    // convenience functions to replace ::exists & ::getSingleOrEmpty pattern
    operator bool() const noexcept {
      return exists;
    }
    const std::string& operator*() const noexcept {
      return value ? *value : empty_string;
    }
    const std::string* operator->() const noexcept {
      return &(operator*)();
    }
  };
  [[nodiscard]] SingleOrNullptrResult getSingleOrNullptr(
      HTTPHeaderCode code) const noexcept;
  [[nodiscard]] SingleOrNullptrResult getSingleOrNullptr(
      folly::StringPiece name) const noexcept;

  /**
   * Returns the value of the header if it's found in the message and is the
   * only value under the given name. If either of these is violated, returns
   * empty_string.
   */
  [[nodiscard]] const std::string& getSingleOrEmpty(HTTPHeaderCode code) const;
  [[nodiscard]] const std::string& getSingleOrEmpty(
      folly::StringPiece name) const;
  [[nodiscard]] const std::string rawGet(const std::string& header) const {
    return getSingleOrEmpty(header);
  }

  /**
   * Get the number of values corresponding to a given header name.
   */
  [[nodiscard]] size_t getNumberOfValues(HTTPHeaderCode code) const;
  [[nodiscard]] size_t getNumberOfValues(folly::StringPiece name) const;

  /**
   * Process the ordered list of values for the given header name:
   * for each value, the function/functor/lambda-expression given as the second
   * parameter will be executed. It should take one const string & parameter
   * and return bool (false to keep processing, true to stop it). Example use:
   *     hdrs.forEachValueOfHeader("someheader", [&] (const std::string& val) {
   *       std::cout << val;
   *       return false;
   *     });
   * This method returns true if processing was stopped (by func returning
   * true), and false otherwise.
   */
  using ForEachValueOfHeaderFnT = std::function<bool(const std::string&)>;
  bool forEachValueOfHeader(folly::StringPiece name,
                            const ForEachValueOfHeaderFnT& func) const;
  bool forEachValueOfHeader(HTTPHeaderCode code,
                            const ForEachValueOfHeaderFnT& func) const;

  /**
   * Remove all instances of the given header, returning true if anything was
   * removed and false if this header didn't exist in our set.
   */
  bool remove(folly::StringPiece name);
  bool remove(HTTPHeaderCode code);
  void rawRemove(const std::string& name) {
    remove(name);
  }

  /**
   * Remove all possible versions of header eg. if x-y-z is the
   * argument it will remove x-y_z, x_y-z and x_y_z too.
   */
  bool removeAllVersions(HTTPHeaderCode code, folly::StringPiece name);

  /**
   * Remove all headers.
   */
  void removeAll();

  /**
   * Remove per-hop-headers and headers named in the Connection header
   * and place the value in strippedHeaders.
   *
   * Also optionally strips the Priority header.
   * The Priority header is defined as end-to-end in the RFC, but a proxy that
   * coalesces requests from multiple downstream connections over the same
   * upstream connection may want to use a hop-by-hop semantics.
   */
  void stripPerHopHeaders(HTTPHeaders& strippedHeaders,
                          bool stripPriority,
                          const HTTPHeaders* customPerHopHeaders);

  /**
   * Get the total number of headers.
   */
  [[nodiscard]] size_t size() const;

  /**
   * Copy all headers from this to hdrs.
   */
  void copyTo(HTTPHeaders& hdrs) const;

  /**
   * Determines whether header with a given code is a per-hop header,
   * which should be stripped by stripPerHopHeaders().
   */
  static std::bitset<256>& perHopHeaderCodes();

 private:
  std::unique_ptr<uint8_t[]> memory_;
  size_t length_{0};
  size_t capacity_{0};
  size_t deletedCount_;

  void copyFrom(const HTTPHeaders& hdrs);

  [[nodiscard]] HTTPHeaderCode* codes() const noexcept;
  [[nodiscard]] HTTPHeaderCode* codes(const uint8_t* memory,
                                      size_t capacity) const noexcept;
  [[nodiscard]] std::string** names() const noexcept;

  [[nodiscard]] std::string** names(const uint8_t* memory,
                                    size_t capacity) const noexcept;
  [[nodiscard]] std::string* values() const noexcept;
  [[nodiscard]] std::string* values(const uint8_t* memory,
                                    size_t) const noexcept;

  /**
   * Moves the named header and values from this group to the destination
   * group.  No-op if the header doesn't exist.  Returns true if header(s) were
   * moved.
   */
  bool transferHeaderIfPresent(folly::StringPiece name, HTTPHeaders& dest);

  // deletes the strings in headerNames_ that we own
  void disposeOfHeaderNames();

  void destroy();

  void ensure(size_t minCapacity);

  void resize(size_t capacity);

  void emplace_back(HTTPHeaderCode code,
                    std::string* name,
                    std::string&& value);
};

template <typename T>
std::string HTTPHeaders::combine(const T& header,
                                 const std::string_view separator) const {
  std::string combined;
  forEachValueOfHeader(header, [&](const std::string& value) -> bool {
    if (combined.empty()) {
      combined.append(value);
    } else {
      combined.append(separator).append(value);
    }
    return false;
  });
  return combined;
}

#ifndef PROXYGEN_HTTPHEADERS_IMPL
#undef ITERATE_OVER_CODES
#undef ITERATE_OVER_STRINGS
#endif // PROXYGEN_HTTPHEADERS_IMPL

} // namespace proxygen
