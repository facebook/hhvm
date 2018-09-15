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

#include "hphp/runtime/ext/extension.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/timer.h"

namespace HPHP {
namespace {

  THREAD_LOCAL(int64_t, request_time_ns);

  int64_t HHVM_FUNCTION(HH_request_time_ns) {
    return *request_time_ns.get();
  }

  struct TimeExtension final : Extension {

    TimeExtension() : Extension("hsl_time", "1.0") {}

    void moduleInit() override {
      HHVM_FALIAS(
        HH\\Lib\\_Private\\Native\\request_time_ns,
        HH_request_time_ns
      );
      loadSystemlib();
    }

    void requestInit() override {
      *request_time_ns.get() = gettime_ns(CLOCK_REALTIME);
    }

  } s_time_extension;

} // anonymous namespace
} // namespace HPHP
