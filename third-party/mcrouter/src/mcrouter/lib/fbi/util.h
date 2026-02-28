/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#ifndef ACCESS_ONCE
#define ACCESS_ONCE(x) (*((volatile __typeof__(x)*)&(x)))
#endif
