/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#ifdef _WIN32

#define HAVE_BACKTRACE
#define HAVE_BACKTRACE_SYMBOLS

extern "C" {

typedef struct _EXCEPTION_POINTERS* LPEXCEPTION_POINTERS;

size_t backtrace(void** frames, size_t n_frames);
char** backtrace_symbols(void** array, size_t n_frames);
size_t backtrace_from_exception(
    LPEXCEPTION_POINTERS exception,
    void** frames,
    size_t n_frames);
}

#endif
