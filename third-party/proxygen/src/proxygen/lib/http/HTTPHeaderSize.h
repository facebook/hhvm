/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>

namespace proxygen {

/**
 * A structure that encapsulates byte counters related to the HTTP headers.
 */
struct HTTPHeaderSize {
  /**
   * The number of bytes used to represent the header after compression or
   * before decompression. If header compression is not supported, the value
   * is set to 0.
   */
  uint32_t compressed{0};

  /**
   * The number of bytes used to represent the serialized header before
   * compression or after decompression, in plain-text format.
   */
  uint32_t uncompressed{0};

  /**
   * The number of bytes encoded as a compressed header block.
   * Header compression algorithms generate a header block plus some control
   * information. The `compressed` field accounts for both. So the control
   * information size can be computed as `compressed` - `compressedBlock`
   */
  uint32_t compressedBlock{0};
};

} // namespace proxygen
