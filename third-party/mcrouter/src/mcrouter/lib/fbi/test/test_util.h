/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>

double measure_time(std::function<void(void)> f);
double measure_time_concurrent(
    unsigned thread_count,
    std::function<void(unsigned)> f);
