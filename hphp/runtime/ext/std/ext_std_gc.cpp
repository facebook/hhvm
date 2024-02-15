/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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
#include "hphp/runtime/ext/std/ext_std.h"
#include "hphp/util/alloc.h"

namespace HPHP {

static bool HHVM_FUNCTION(gc_enabled) {
  return tl_heap->isGCEnabled();
}

static void HHVM_FUNCTION(gc_enable) {
  tl_heap->setGCEnabled(true);
}

static void HHVM_FUNCTION(gc_disable) {
  tl_heap->setGCEnabled(false);
}

static int64_t HHVM_FUNCTION(gc_collect_cycles) {
  tl_heap->collect("gc_collect_cycles");
  return 0; // seriously, count cycles?
}

static int64_t HHVM_FUNCTION(gc_mem_caches) {
  flush_thread_caches();
  return 0;
}

static void HHVM_FUNCTION(gc_check_heap) {
  tl_heap->checkHeap("gc_check_heap");
}

void StandardExtension::registerNativeGc() {
  HHVM_FE(gc_enabled);
  HHVM_FE(gc_enable);
  HHVM_FE(gc_disable);
  HHVM_FE(gc_collect_cycles);
  HHVM_FE(gc_mem_caches);
  HHVM_FE(gc_check_heap);
}

}
