/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>

namespace facebook {
namespace memcache {

/**
 * Validates the qosClass and qosPath sets appropriate qos if valid.
 * Returns true on success and false otherwise.
 *
 */
bool getQoS(uint64_t qosClassLvl, uint64_t qosPathLvl, uint64_t& qos);

} // namespace memcache
} // namespace facebook
