/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <memory>

namespace facebook {
namespace memcache {

class HostInfo;
using HostInfoPtr = std::shared_ptr<HostInfo>;

} // namespace memcache
} // namespace facebook
