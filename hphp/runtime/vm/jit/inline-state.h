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

#include <vector>

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/srckey.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct Block;

struct InlineFrame {
  /*
   * Saved caller's SrcKey at which we entered inlining.
   */
  SrcKey callerSk;

  /*
   * Block that will serve as a branch target for returning to the caller.
   */
  Block* returnTarget;

  /*
   * Block that will suspend the inlined frame.
   */
  Block* suspendTarget;

  /*
   * Block that will side exit from the inlined frame, assuming the preparation
   * for side exit has commenced.
   */
  Block* sideExitTarget;

  /*
   * Block that will finish the catch block, assuming the evaluation stack
   * of the callee is empty. Without and with locals already decrefd.
   */
  Block* endCatchTarget;
  Block* endCatchLocalsDecRefdTarget;

  /*
   * Offset from FCall to return control to if the callee finished eagerly.
   */
  Offset asyncEagerOffset;

  /*
   * Saved cost.
   */
  int savedCost;
};

struct InlineState {
  /*
   * Translating a unit just to compute the cost of inlining.
   */
  bool conjure{false};

  /*
   * Costs associated with inlining.
   */
  int cost{0};
  int stackDepth{0};

  /*
   * Inline state for each inlined frame. The last one is for the current frame.
   */
  std::vector<InlineFrame> frames;
};

///////////////////////////////////////////////////////////////////////////////
}}
