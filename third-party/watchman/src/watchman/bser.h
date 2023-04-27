/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fmt/core.h>
#include "watchman/thirdparty/jansson/jansson.h"

typedef struct bser_ctx {
  uint32_t bser_version;
  uint32_t bser_capabilities;
  json_dump_callback_t dump;
} bser_ctx_t;

class BserParseError : public std::exception {
 public:
  const json_error_t detail;

  explicit BserParseError(const char* what) : detail{what} {}

  template <typename... T>
  explicit BserParseError(fmt::format_string<T...> fmt, T&&... args)
      : detail{fmt::format(fmt, std::forward<T>(args)...).c_str()} {
    // TODO: this constructor could use fmt::format_to_n to avoid an extra copy
    // Or perhaps json_error_t should use std::string instead of fixed-size
    // arrays.
  }

  explicit BserParseError(const json_error_t& d) : detail{d} {}

  const char* what() const noexcept override {
    return detail.text;
  }

 private:
  mutable std::string what_;
};

class BserParseTooDeep : public BserParseError {
 public:
  BserParseTooDeep()
      : BserParseError{"bser document exceeds recursion limit"} {}
};

#define BSER_MAGIC "\x00\x01"
#define BSER_V2_MAGIC "\x00\x02"

// BSERv2 capabilities. Must be powers of 2.
#define BSER_CAP_DISABLE_UNICODE 0x1
#define BSER_CAP_DISABLE_UNICODE_FOR_ERRORS 0x2

int w_bser_write_pdu(
    const uint32_t bser_version,
    const uint32_t capabilities,
    json_dump_callback_t dump,
    const json_ref& json,
    void* data);
int w_bser_dump(const bser_ctx_t* ctx, const json_ref& json, void* data);

constexpr size_t kDecodeIntFailed = ~size_t{};

/**
 * Attempt to unserialize an integer value.
 * Returns the integer if successful. Returns std::nullopt if unsuccessful.
 * If decoding fails, *needed is set to the number of bytes required to parse
 * the integer.
 *
 * If *needed is kDecodeIntFailed, then `buf` does not contain a valid BSER int.
 */
std::optional<json_int_t>
bunser_int(const char* buf, size_t avail, size_t* needed);

/**
 * Parses a value from a BSER document.
 *
 * Ignores any unused data at the end of the buffer.
 */
json_ref bunser(const char* buf, const char* end);
