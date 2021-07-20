/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once

#define SD_LEN    8
#define SD_DATA   16
#define SD_HASH   12

// Under certain conditions, we implement both StringData::hash and certain
// hash table lookup methods in x86 assembly.
#if defined(__SSE4_2__) && !defined(NO_HWCRC) && \
    !defined(__APPLE__) && !defined(_MSC_VER)
#define USE_X86_STRING_HELPERS
#endif

// Under certain conditions, we implement StringData::hash in ARM assembly.
#if defined(ENABLE_AARCH64_CRC) && !defined(NO_HWCRC) && \
    !defined(__APPLE__) && !defined(_MSC_VER)
#define USE_ARM_STRING_HELPERS
#endif
