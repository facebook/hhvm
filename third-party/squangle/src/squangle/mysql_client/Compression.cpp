/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <glog/logging.h>
#include <algorithm>
#include <optional>
#include <unordered_map>

#include "squangle/mysql_client/Compression.h"

namespace facebook::common::mysql_client {

bool setCompressionOption(MYSQL* mysql, CompressionAlgorithm algo) {
#if MYSQL_VERSION_ID < 80018
  auto comp_lib = getCompressionValue(algo);
  auto res = mysql_options(mysql, MYSQL_OPT_COMP_LIB, (void*)&comp_lib);
#else
  auto res = mysql_options(
      mysql, MYSQL_OPT_COMPRESSION_ALGORITHMS, getCompressionName(algo).data());
#endif
  return res == 0; // 0 indicates success
}

#if MYSQL_VERSION_ID < 80018
static const std::unordered_map<CompressionAlgorithm, mysql_compression_lib>
    compressionAlgorithms = {
        {ZLIB, MYSQL_COMPRESSION_ZLIB},
        {ZSTD, MYSQL_COMPRESSION_ZSTD},
        {ZSTD_STREAM, MYSQL_COMPRESSION_ZSTD_STREAM},
        {LZ4F_STREAM, MYSQL_COMPRESSION_LZ4F_STREAM},
};

mysql_compression_lib getCompressionValue(CompressionAlgorithm algo) {
  auto it = compressionAlgorithms.find(algo);
  CHECK(it != compressionAlgorithms.end())
      << "Invalid compression algorithm enum: " << (int)algo;

  return it->second;
}
#else
// These strings _must_ match the string supported in MySQL
// (https://fburl.com/diffusion/5jkzvbg0)
static const std::unordered_map<CompressionAlgorithm, std::string>
    compressionAlgorithms = {
        {ZLIB, "zlib"},
        {ZSTD, "zstd"},
        {ZSTD_STREAM, "zstd_stream"},
        {LZ4F_STREAM, "lz4f_stream"},
};

std::optional<CompressionAlgorithm> parseCompressionName(
    std::string_view name) {
  auto it = std::find_if(
      compressionAlgorithms.begin(),
      compressionAlgorithms.end(),
      [&](const auto& entry) { return entry.second == name; });

  if (it == compressionAlgorithms.end()) {
    return std::nullopt;
  }

  return it->first;
}

const std::string& getCompressionName(CompressionAlgorithm algo) {
  auto it = compressionAlgorithms.find(algo);
  CHECK(it != compressionAlgorithms.end())
      << "Invalid compression algorithm enum: " << (int)algo;

  return it->second;
}
#endif

} // namespace facebook::common::mysql_client
