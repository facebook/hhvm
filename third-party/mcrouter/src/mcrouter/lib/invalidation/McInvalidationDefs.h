/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

namespace facebook {
namespace memcache {

inline const std::string kMcDeleteReqAttrInvalidationTimeout =
    "invalidation_timeout";
inline const std::string kMcDeleteReqAttrDBLogTimestamp = "dblog_timestamp";
inline const std::string kMcDeleteReqAttrSource = "source";
inline const std::string kMcDeleteReqHlc = "hlc";

enum class McDeleteRequestSource : uint8_t {
  UNKNOWN,
  INGESTION_SERVICE,
  GLOSTIC,
  FAILED_INVALIDATION,
  CROSS_REGION_BROADCAST_INVALIDATION,
  CROSS_REGION_DIRECTED_INVALIDATION,
};

} // namespace memcache
} // namespace facebook
