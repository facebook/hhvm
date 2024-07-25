/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/MultiBackendFactory.h>

namespace fizz {

// TODO: This needs to be wired up from fizz-config.h

/**
 * DefaultFactory is a type alias that points to a concrete, non-abstract
 * Factory instance that can be used by various internal components of Fizz
 * to get the "default compile time configured" Factory.
 */
using DefaultFactory = ::fizz::MultiBackendFactory;

} // namespace fizz
