/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <variant>
#include <vector>

#include "squangle/mysql_client/InternalConnection.h"

namespace facebook::common::mysql_client::test {

/**
 * MockColumnValue represents a single column value in a mock row.
 * Uses std::variant to hold different types that MySQL columns can represent.
 */
using MockColumnValue = std::variant<
    std::monostate, // NULL
    bool,
    int64_t,
    uint64_t,
    double,
    std::string>;

/**
 * MockInternalRow implements InternalRow for unit testing without MySQL.
 *
 * Usage:
 *   std::vector<MockColumnValue> values = {"John", int64_t{25}, 3.14};
 *   auto row = std::make_unique<MockInternalRow>(std::move(values));
 *   EXPECT_EQ(row->columnString(0), "John");
 *   EXPECT_EQ(row->columnInt64(1), 25);
 */
class MockInternalRow : public InternalRow {
 public:
  explicit MockInternalRow(std::vector<MockColumnValue> values)
      : values_(std::move(values)) {}

  folly::StringPiece columnString(size_t col) const override {
    if (col >= values_.size()) {
      return folly::StringPiece();
    }
    if (auto* str = std::get_if<std::string>(&values_[col])) {
      return folly::StringPiece(*str);
    }
    return folly::StringPiece();
  }

  bool columnBool(size_t col) const override {
    if (col >= values_.size()) {
      return false;
    }
    if (auto* b = std::get_if<bool>(&values_[col])) {
      return *b;
    }
    if (auto* i = std::get_if<int64_t>(&values_[col])) {
      return *i != 0;
    }
    return false;
  }

  int64_t columnInt64(size_t col) const override {
    if (col >= values_.size()) {
      return 0;
    }
    if (auto* i = std::get_if<int64_t>(&values_[col])) {
      return *i;
    }
    if (auto* u = std::get_if<uint64_t>(&values_[col])) {
      return static_cast<int64_t>(*u);
    }
    return 0;
  }

  uint64_t columnUInt64(size_t col) const override {
    if (col >= values_.size()) {
      return 0;
    }
    if (auto* u = std::get_if<uint64_t>(&values_[col])) {
      return *u;
    }
    if (auto* i = std::get_if<int64_t>(&values_[col])) {
      return static_cast<uint64_t>(*i);
    }
    return 0;
  }

  double columnDouble(size_t col) const override {
    if (col >= values_.size()) {
      return 0.0;
    }
    if (auto* d = std::get_if<double>(&values_[col])) {
      return *d;
    }
    return 0.0;
  }

  Type columnType(size_t col) const override {
    if (col >= values_.size()) {
      return Type::Null;
    }
    return std::visit(
        [](auto&& arg) -> Type {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, std::monostate>) {
            return Type::Null;
          } else if constexpr (std::is_same_v<T, bool>) {
            return Type::Bool;
          } else if constexpr (std::is_same_v<T, int64_t>) {
            return Type::Int64;
          } else if constexpr (std::is_same_v<T, uint64_t>) {
            return Type::UInt64;
          } else if constexpr (std::is_same_v<T, double>) {
            return Type::Double;
          } else if constexpr (std::is_same_v<T, std::string>) {
            return Type::String;
          }
          return Type::Null;
        },
        values_[col]);
  }

  size_t columnLength(size_t col) const override {
    if (col >= values_.size()) {
      return 0;
    }
    return std::visit(
        [](auto&& arg) -> size_t {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, std::monostate>) {
            return 0;
          } else if constexpr (std::is_same_v<T, std::string>) {
            return arg.size();
          } else {
            return sizeof(T);
          }
        },
        values_[col]);
  }

 private:
  std::vector<MockColumnValue> values_;
};

} // namespace facebook::common::mysql_client::test
