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

#ifndef HPHP_LAYOUT_SELECTION_H_
#define HPHP_LAYOUT_SELECTION_H_

#include "hphp/runtime/vm/jit/array-layout.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/runtime/base/bespoke/logging-profile.h"

namespace HPHP { namespace bespoke {

using LayoutWeightVector =
  std::vector<std::pair<const StructLayout*, double>>;

// Layout selection. We'll call selectBespokeLayouts in RTA, and then we'll
// be able to use layoutForSource / layoutForSink for optimized code.
jit::ArrayLayout layoutForSource(SrcKey sk);
SinkLayout layoutForSink(const jit::TransIDSet& ids, SrcKey sk);
void selectBespokeLayouts();

}}

#endif // HPHP_LAYOUT_SELECTION_H_
