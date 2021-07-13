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


#include <folly/Singleton.h>

#include "hphp/runtime/vm/taint.h"

#include "hphp/util/trace.h"

namespace HPHP {
namespace taint {

namespace {
  struct SingletonTag {};
}

folly::Singleton<State, SingletonTag> singleton{};
/* static */ std::shared_ptr<State> State::get() {
  return singleton.try_get();
}

TRACE_SET_MOD(taint);

void retC() {
  FTRACE(1, "taint: RetC\n");
  State::get()->history.push_back(1);
}

} // namespace taint
} // namespace HPHP
