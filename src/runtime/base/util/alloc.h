/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_ALLOC_H__
#define __HPHP_ALLOC_H__

#include <stdlib.h>

#include <google/malloc_extension.h>
#include <jemalloc/jemalloc.h>

extern "C" {
#define MallocExtensionInstance _ZN15MallocExtension8instanceEv
  MallocExtension* MallocExtensionInstance() __attribute__((weak));

  int mallctl(const char *name, void *oldp, size_t *oldlenp, void *newp,
          size_t newlen) __attribute__((weak));
  void malloc_stats_print(void (*write_cb)(void *, const char *),
          void *cbopaque, const char *opts) __attribute__((weak));
}

namespace HPHP { namespace Util {
///////////////////////////////////////////////////////////////////////////////

/**
 * Safe memory allocation.
 */
  void* safe_malloc(size_t size);
  void* safe_realloc(void* ptr, size_t size);
  void  safe_free(void* ptr);

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_ALLOC_H__
