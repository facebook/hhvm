/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

// From https://gcc.gnu.org/wiki/Visibility
#if defined _WIN32 || defined __CYGWIN__
// The current uses of this doesn't actually
// have any need to be present on Windows,
// and building proxygen as a dynamic lib
// under Windows is not currently supported.
// HHVM builds Proxygen as a static library.
#define FB_EXPORT
#define FB_LOCAL
#else
#if __GNUC__ >= 4
#define FB_EXPORT __attribute__((visibility("default")))
#define FB_LOCAL __attribute__((visibility("hidden")))
#else
#define FB_EXPORT
#define FB_LOCAL
#endif
#endif
