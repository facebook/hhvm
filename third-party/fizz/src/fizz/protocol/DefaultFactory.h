/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/fizz-config.h>

#include FIZZ_DEFAULT_FACTORY_HEADER

namespace fizz {
/**
 * DefaultFactory is a type alias that points to a concrete, non-abstract
 * Factory instance that can be used by various internal components of Fizz
 * to get the "default compile time configured" Factory.
 */
using DefaultFactory = FIZZ_DEFAULT_FACTORY;

} // namespace fizz
