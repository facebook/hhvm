/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <algorithm>
#include <boost/variant.hpp>
#include <cstdint>
#include <folly/Range.h>
#include <folly/String.h>
#include <functional>
#include <glog/logging.h>
#include <iostream>
#include <proxygen/lib/http/HTTPCommonHeaders.h>
#include <string>

namespace proxygen {

/*
 * HPACKHeaderName stores the header name of HPACKHeaders. If
 * the header name is in HTTPCommonHeader, it will store a
 * pointer to the common header, otherwise, it will store a
 * pointer to a the dynamically allocated std::string
 */
class HPACKHeaderName {
 public:
  HPACKHeaderName() {
  }

  explicit HPACKHeaderName(folly::StringPiece name) {
    storeAddress(name);
  }
  explicit HPACKHeaderName(HTTPHeaderCode headerCode) {
    CHECK_NE(headerCode, HTTP_HEADER_NONE);
    CHECK_NE(headerCode, HTTP_HEADER_OTHER);
    address_ = HTTPCommonHeaders::getPointerToName(
        headerCode, HTTPCommonHeaderTableType::TABLE_LOWERCASE);
  }
  HPACKHeaderName(const HPACKHeaderName& headerName) {
    copyAddress(headerName);
  }
  HPACKHeaderName(HPACKHeaderName&& goner) noexcept {
    moveAddress(goner);
  }
  HPACKHeaderName& operator=(folly::StringPiece name) {
    resetAddress();
    storeAddress(name);
    return *this;
  }
  HPACKHeaderName& operator=(const HPACKHeaderName& headerName) {
    if (this != &headerName) {
      resetAddress();
      copyAddress(headerName);
    }
    return *this;
  }
  HPACKHeaderName& operator=(HPACKHeaderName&& goner) noexcept {
    if (this != &goner) {
      resetAddress();
      moveAddress(goner);
    }
    return *this;
  }

  ~HPACKHeaderName() {
    resetAddress();
  }

  /*
   * Compare the strings stored in HPACKHeaderName
   */
  bool operator==(const HPACKHeaderName& headerName) const {
    return address_ == headerName.address_ ||
           *address_ == *(headerName.address_);
  }
  bool operator!=(const HPACKHeaderName& headerName) const {
    // Utilize the == overloaded operator
    return !(*this == headerName);
  }
  bool operator>(const HPACKHeaderName& headerName) const {
    if (!isAllocated() && !headerName.isAllocated()) {
      // Common header tables are aligned alphabetically (unit tested as well
      // to ensure it isn't accidentally changed)
      return address_ > headerName.address_;
    } else {
      return *address_ > *(headerName.address_);
    }
  }
  bool operator<(const HPACKHeaderName& headerName) const {
    if (!isAllocated() && !headerName.isAllocated()) {
      // Common header tables are aligned alphabetically (unit tested as well
      // to ensure it isn't accidentally changed)
      return address_ < headerName.address_;
    } else {
      return *address_ < *(headerName.address_);
    }
  }
  bool operator>=(const HPACKHeaderName& headerName) const {
    // Utilize existing < overloaded operator
    return !(*this < headerName);
  }
  bool operator<=(const HPACKHeaderName& headerName) const {
    // Utilize existing > overload operator
    return !(*this > headerName);
  }

  /*
   * Return std::string stored in HPACKHeaderName
   */
  const std::string& get() const {
    return *address_;
  }

  /*
   * Returns the HTTPHeaderCode associated with the wrapped address_
   */
  HTTPHeaderCode getHeaderCode() const {
    return HTTPCommonHeaders::getCodeFromTableName(
        address_, HTTPCommonHeaderTableType::TABLE_LOWERCASE);
  }

  /*
   * Returns whether the name pointed to by this instance is a common header
   */
  bool isCommonHeader() const {
    return HTTPCommonHeaders::isNameFromTable(
        address_, HTTPCommonHeaderTableType::TABLE_LOWERCASE);
  }

  /*
   * Exposing wrapped std::string member properties
   */
  uint32_t size() const {
    return (uint32_t)(address_->size());
  }
  const char* c_str() const {
    return address_->c_str();
  }

 private:
  /*
   * Store a reference to either a common header or newly allocated string
   */
  void storeAddress(folly::StringPiece name) {
    HTTPHeaderCode headerCode =
        HTTPCommonHeaders::hash(name.data(), name.size());
    if (headerCode == HTTPHeaderCode::HTTP_HEADER_NONE ||
        headerCode == HTTPHeaderCode::HTTP_HEADER_OTHER) {
      auto newAddress = new std::string(name);
      folly::toLowerAscii(*newAddress);
      address_ = newAddress;
    } else {
      address_ = HTTPCommonHeaders::getPointerToName(
          headerCode, HTTPCommonHeaderTableType::TABLE_LOWERCASE);
    }
  }

  /*
   * Copy the address_ from another HPACKHeaderName
   */
  void copyAddress(const HPACKHeaderName& headerName) {
    if (headerName.isAllocated()) {
      address_ = new std::string(headerName.get());
    } else {
      address_ = headerName.address_;
    }
  }

  /*
   * Move the address_ from another HPACKHeaderName
   */
  void moveAddress(HPACKHeaderName& goner) {
    address_ = goner.address_;
    goner.address_ = nullptr;
  }

  /*
   * Delete any allocated memory and reset address_ to nullptr
   */
  void resetAddress() {
    if (isAllocated()) {
      delete address_;
    }
    address_ = nullptr;
  }

  /*
   * Returns whether the underlying address_ points to a string that was
   * allocated (memory) by this instance
   */
  bool isAllocated() const {
    if (address_ == nullptr) {
      return false;
    } else {
      return !HTTPCommonHeaders::isNameFromTable(
          address_, HTTPCommonHeaderTableType::TABLE_LOWERCASE);
    }
  }

  /*
   * Address either stores a pointer to a header name in HTTPCommonHeaders,
   * or stores a pointer to a dynamically allocated std::string
   */
  const std::string* address_ = nullptr;
};

inline std::ostream& operator<<(std::ostream& os, const HPACKHeaderName& name) {
  os << name.get();
  return os;
}

} // namespace proxygen

namespace std {

template <>
struct hash<proxygen::HPACKHeaderName> {
  size_t operator()(const proxygen::HPACKHeaderName& headerName) const {
    return std::hash<std::string>()(headerName.get());
  }
};

} // namespace std
