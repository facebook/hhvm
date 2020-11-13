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

#include "hphp/runtime/base/bespoke/bespoke-top.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/punt.h"

namespace HPHP { namespace bespoke {

using namespace jit;
using namespace jit::irgen;

namespace {
BespokeTop* s_layout;
}

BespokeTop::BespokeTop(): Layout("BespokeTop", {}, true) {}

void BespokeTop::InitializeLayouts() {
  s_layout = new BespokeTop();
}

LayoutIndex BespokeTop::GetLayoutIndex() {
  return s_layout->index();
}

}}
