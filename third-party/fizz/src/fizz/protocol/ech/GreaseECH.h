/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/Factory.h>
#include <fizz/protocol/ech/ECHExtensions.h>
#include <fizz/protocol/ech/GreaseECHSetting.h>

namespace fizz {
namespace ech {
OuterECHClientHello generateGreaseECH(
    const GreaseECHSetting& setting,
    const Factory& factory,
    size_t encodedChloSize);
} // namespace ech
} // namespace fizz
