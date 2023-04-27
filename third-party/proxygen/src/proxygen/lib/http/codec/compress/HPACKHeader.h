/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Conv.h>
#include <folly/FBString.h>
#include <glog/logging.h>
#include <ostream>
#include <proxygen/lib/http/codec/compress/HPACKHeaderName.h>
#include <string>

namespace proxygen {

class HPACKHeader {
 public:
  static const uint32_t kMinLength = 32;

  HPACKHeader() {
  }

  HPACKHeader(const HPACKHeaderName& name_, folly::StringPiece value_)
      : name(name_), value(value_.data(), value_.size()) {
  }

  // this one is usually with a code
  HPACKHeader(const HPACKHeaderName& name_, folly::fbstring&& value_)
      : name(name_), value(std::move(value_)) {
  }

  HPACKHeader(HPACKHeaderName&& name_, folly::fbstring&& value_)
      : name(std::move(name_)), value(std::move(value_)) {
  }

  HPACKHeader(HPACKHeaderName&& name_, folly::StringPiece value_)
      : name(std::move(name_)), value(value_.data(), value_.size()) {
  }

  HPACKHeader(folly::StringPiece name_, folly::StringPiece value_)
      : name(name_), value(value_.data(), value_.size()) {
  }

  HPACKHeader(HPACKHeader&& goner) noexcept
      : name(std::move(goner.name)), value(std::move(goner.value)) {
  }

  HPACKHeader& operator=(HPACKHeader&& goner) noexcept {
    std::swap(name, goner.name);
    std::swap(value, goner.value);
    return *this;
  }

  ~HPACKHeader() {
  }

  /**
   * size of usable bytes of the header entry, does not include overhead
   */
  uint32_t realBytes() const {
    return realBytes(name.size(), value.size());
  }

  static uint32_t realBytes(uint64_t nameSize, uint64_t valueSize) {
    DCHECK_LE(nameSize + valueSize, std::numeric_limits<uint32_t>::max());
    return folly::tryTo<uint32_t>(nameSize + valueSize)
        .value_or(std::numeric_limits<uint32_t>::max());
  }

  /**
   * size in bytes of the header entry, as defined in the HPACK spec
   */
  uint32_t bytes() const {
    return kMinLength + realBytes();
  }

  static uint32_t bytes(uint64_t nameSize, uint64_t valueSize) {
    DCHECK_LE(kMinLength + nameSize + valueSize,
              std::numeric_limits<uint32_t>::max());
    return folly::tryTo<uint32_t>(kMinLength + realBytes(nameSize, valueSize))
        .value_or(std::numeric_limits<uint32_t>::max());
  }

  bool operator==(const HPACKHeader& other) const {
    return name == other.name && value == other.value;
  }
  bool operator<(const HPACKHeader& other) const {
    bool eqname = (name == other.name);
    if (!eqname) {
      return name < other.name;
    }
    return value < other.value;
  }
  bool operator>(const HPACKHeader& other) const {
    bool eqname = (name == other.name);
    if (!eqname) {
      return name > other.name;
    }
    return value > other.value;
  }

  HPACKHeader copy() const {
    return HPACKHeader(name, value);
  }

  /**
   * Some header entries don't have a value, see StaticHeaderTable
   */
  bool hasValue() const {
    return !value.empty();
  }

  HPACKHeaderName name;
  folly::fbstring value;
};

std::ostream& operator<<(std::ostream& os, const HPACKHeader& h);

} // namespace proxygen
