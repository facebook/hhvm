/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP {

static bool HHVM_FUNCTION(gc_enabled) {
  return RuntimeOption::EvalEnableGC;
}

static void HHVM_FUNCTION(gc_enable) {
  // maybe this should turn on/off gc for just this request?
}

static void HHVM_FUNCTION(gc_disable) {
  // maybe this should turn on/off gc for just this request?
}

static int64_t HHVM_FUNCTION(gc_collect_cycles) {
  MM().collect("gc_collect_cycles");
  return 0; // seriously, count cycles?
}

static void HHVM_FUNCTION(gc_check_heap) {
  MM().checkHeap("gc_check_heap");
}

void StandardExtension::initGc() {
  HHVM_FE(gc_enabled);
  HHVM_FE(gc_enable);
  HHVM_FE(gc_disable);
  HHVM_FE(gc_collect_cycles);
  HHVM_FE(gc_check_heap);
  loadSystemlib("std_gc");
}

}
