/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>
#include <unordered_map>

namespace facebook {
namespace common {
namespace mysql_client {

using QueryRespAttrs = std::unordered_map<std::string, std::string>;
}
} // namespace common
} // namespace facebook
