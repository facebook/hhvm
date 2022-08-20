/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>

#ifdef _WIN32

// DWORD = uint32_t

const char* win32_strerror(uint32_t err);
int map_win32_err(uint32_t err);

#endif
