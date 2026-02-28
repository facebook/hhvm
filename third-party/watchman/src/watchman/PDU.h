/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>
#include "watchman/Result.h"
#include "watchman/thirdparty/jansson/jansson.h"

namespace watchman {

class Stream;

enum PduType : uint32_t {
  need_data,
  is_json_compact,
  is_json_pretty,
  is_bser,
  is_bser_v2
};

// Required for fmt 10
inline uint32_t format_as(PduType type) {
  return static_cast<uint32_t>(type);
}

/**
 * Specifies the wire encoding of a Watchman request or response.
 *
 * This could be made to fit in 8 bits, but it doesn't matter.
 */
struct PduFormat {
  PduType type = need_data;
  /// Capability bits only used for BSER v2, defined in bser.h
  uint32_t capabilities = 0;
};

class PduBuffer {
 public:
  char* buf;
  uint32_t allocd = 0;
  uint32_t rpos = 0;
  uint32_t wpos = 0;

  /// The encoding format detected by decodeNext
  PduFormat format;

  PduBuffer();
  PduBuffer(const PduBuffer&) = delete;
  PduBuffer(PduBuffer&&) = delete;
  PduBuffer& operator=(const PduBuffer&) = delete;
  PduBuffer& operator=(const PduBuffer&&) = delete;
  ~PduBuffer();

  void clear();
  ResultErrno<folly::Unit>
  jsonEncodeToStream(const json_ref& json, Stream* stm, int flags);
  ResultErrno<folly::Unit> bserEncodeToStream(
      uint32_t bser_version,
      uint32_t bser_capabilities,
      const json_ref& json,
      Stream* stm);

  ResultErrno<folly::Unit>
  pduEncodeToStream(PduFormat format, const json_ref& json, Stream* stm);

  std::optional<json_ref> decodeNext(Stream* stm, json_error_t* jerr);

  bool readAndDetectPdu(Stream* stm, json_error_t* jerr);
  std::optional<json_ref> decodePdu(Stream* stm, json_error_t* jerr);
  bool streamPdu(Stream* stm, json_error_t* jerr);

 private:
  uint32_t shuntDown();
  bool fillBuffer(Stream* stm);
  PduType detectPdu();
  std::optional<json_ref> readJsonPrettyPdu(Stream* stm, json_error_t* jerr);
  std::optional<json_ref> readJsonPdu(Stream* stm, json_error_t* jerr);
  std::optional<json_ref>
  readBserPdu(Stream* stm, uint32_t bser_version, json_error_t* jerr);
  bool decodePduInfo(
      Stream* stm,
      uint32_t bser_version,
      json_int_t* len,
      json_int_t* bser_capabilities,
      json_error_t* jerr);
  bool streamUntilNewLine(Stream* stm);
  bool streamN(Stream* stm, json_int_t len, json_error_t* jerr);
};

} // namespace watchman
