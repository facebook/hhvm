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

#ifndef HPHP_BESPOKE_TOP_H
#define HPHP_BESPOKE_TOP_H

#include "hphp/runtime/base/bespoke/layout.h"

namespace HPHP { namespace bespoke {

struct BespokeTop : public Layout {
  using SSATmp = jit::SSATmp;
  using Block = jit::Block;
  using IRInstruction = jit::IRInstruction;
  using IRGS = jit::irgen::IRGS;

  static void InitializeLayouts();
  static LayoutIndex GetLayoutIndex();

private:
  BespokeTop();
};

}}

#endif
