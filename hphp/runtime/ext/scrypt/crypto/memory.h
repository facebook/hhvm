/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <folly/portability/Memory.h>

namespace folly_ext {

// Replace this once folly has public access to aligned allocation (intended to
// look something like std::make_unique<>).
inline void* aligned_malloc(size_t size, size_t alignment) {
  return folly::detail::aligned_malloc(size, alignment);
}

inline void aligned_free(void* ptr) {
  folly::detail::aligned_free(ptr);
}

}
