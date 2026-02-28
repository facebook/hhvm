/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <mysql.h> // @manual
#include <optional>
#include <string>

namespace facebook::common::mysql_client {

// Compression algorithms supported by MySQL:
// https://fburl.com/diffusion/kq2fbmr2
enum CompressionAlgorithm {
  ZLIB,
  ZSTD,
  ZSTD_STREAM,
  LZ4F_STREAM,
};

bool setCompressionOption(MYSQL* mysql, CompressionAlgorithm algo);

#if MYSQL_VERSION_ID >= 80000
#if MYSQL_VERSION_ID < 80018
mysql_compression_lib getCompressionValue(CompressionAlgorithm algo);
#else
std::optional<CompressionAlgorithm> parseCompressionName(std::string_view name);
const std::string& getCompressionName(CompressionAlgorithm algo);
#endif
#endif

} // namespace facebook::common::mysql_client
