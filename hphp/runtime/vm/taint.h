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

#ifdef HHVM_TAINT

#include "hphp/runtime/vm/hhbc.h"

#include <memory>

namespace HPHP {
namespace taint {

struct State {
  static std::shared_ptr<State> get();

  // Dummy data for now. Just to illustrate testing.
  void reset() {
    history.clear();
  }
  std::vector<int> history;
};

#define O(name, imm, in, out, flags) \
void iop##name();

OPCODES

#undef O

} // taint
} // HPHP

#endif
