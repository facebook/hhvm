/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/String.h>

#include "mcrouter/tools/mcpiper/PrettyFormat.h"
#include "mcrouter/tools/mcpiper/StyledString.h"

namespace facebook {
namespace memcache {

/**
 * Formats the value part of request/reply.
 */
class ValueFormatter {
 public:
  ValueFormatter() = default;
  virtual ~ValueFormatter() = default;

  // Non-copyable
  ValueFormatter(const ValueFormatter&) = delete;
  ValueFormatter& operator=(const ValueFormatter&) = delete;

  // Non-movable
  ValueFormatter(ValueFormatter&&) = delete;
  ValueFormatter&& operator=(ValueFormatter&&) = delete;

  /**
   * Given a raw MC value and its flags, perform best effort uncompression
   * and formatting.
   *
   * @param value             The value to uncompress and format.
   * @param flags             Flags
   * @param format            Format to use.
   * @param script            If true, produce a machine readable JSON output
   * @param uncompressedSize  Output parameter containing the uncompressed size.
   * @return                  The formatted value.
   */
  virtual StyledString uncompressAndFormat(
      folly::StringPiece value,
      uint64_t /* flags */,
      PrettyFormat format,
      bool /* script */,
      size_t& uncompressedSize) noexcept {
    uncompressedSize = value.size();
    return StyledString(value.str(), format.dataValueColor);
  }
};
} // namespace memcache
} // namespace facebook
