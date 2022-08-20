/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/network/CaretHeader.h"

namespace facebook {
namespace memcache {

enum class ParseStatus {
  Ok,
  MessageParseError,
  NotEnoughData,
};

/**
 * Parses caret message header and fills up the CaretMessageInfo.
 *
 * @param buf   Pointer to buffer.
 * @param nbuf  Length of the buffer.
 * @param info  Output arg with the caret header data.
 *
 * @return status
 */
ParseStatus
caretParseHeader(const uint8_t* buf, size_t nbuf, CaretMessageInfo& info);

/**
 * Prepares the caret message header.
 *
 * @param info          Header info.
 * @param headerBuffer  Pointer to buffer. Buffer must be large enough to
 *                      hold header and extra fields.
 *
 * @return              Number of bytes written to buffer.
 */
size_t caretPrepareHeader(const CaretMessageInfo& info, char* headerBuffer);

} // namespace memcache
} // namespace facebook
