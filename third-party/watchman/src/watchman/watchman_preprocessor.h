/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

// Helpers for pasting __LINE__ for symbol generation
#define w_paste2(pre, post) pre##post
#define w_paste1(pre, post) w_paste2(pre, post)
#define w_gen_symbol(pre) w_paste1(pre, __LINE__)

#if _MSC_VER >= 1400
#include <sal.h> // @manual
#if _MSC_VER > 1400
#define WATCHMAN_FMT_STRING(x) _Printf_format_string_ x
#else
#define WATCHMAN_FMT_STRING(x) __format_string x
#endif
#else
#define WATCHMAN_FMT_STRING(x) x
#endif

#ifdef __GNUC__
#define WATCHMAN_FMT_ATTR(fmt_param_no, dots_param_no) \
  __attribute__((__format__(__printf__, fmt_param_no, dots_param_no)))
#endif

#ifndef WATCHMAN_FMT_ATTR
#define WATCHMAN_FMT_ATTR(fmt_param_no, dots_param_no) /* nothing */
#endif
