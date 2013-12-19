/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HHBBC_OPTIONS_H_
#define incl_HHBBC_OPTIONS_H_

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

struct OptimizerOptions {
  constexpr OptimizerOptions() {}

  /*
   * If true, completely remove jumps to blocks that are inferred to
   * be dead.  When false, dead blocks are replaced with Fatal
   * bytecodes.
   */
  bool RemoveDeadBlocks = false;

  /*
   * Whether to propagate constant values by replacing instructions
   * which are known to always produce a constant with instructions
   * that produce that constant.
   */
  bool ConstantProp = true;

  /*
   * If true, insert opcodes that assert inferred types, so we can
   * assume them at runtime.
   */
  bool InsertAssertions = true;
  bool InsertStackAssertions = true;

  /*
   * If true, try to filter asserts out that are "obvious" (this is a
   * code size optimization).  It can be useful to turn this option
   * off for debugging.
   *
   * Has no effect if !InsertStackAssertions.
   */
  bool FilterAssertions = true;

  /*
   * Whether to replace bytecode with less expensive bytecodes when we
   * can.  E.g. InstanceOf -> InstanceOfD or FPushFunc -> FPushFuncD.
   */
  bool StrengthReduceBC = true;

  //////////////////////////////////////////////////////////////////////
  // Flags below this line perform optimizations that intentionally
  // may have user-visible changes to program behavior.
  //////////////////////////////////////////////////////////////////////

  /*
   * If true, we'll propagate global constants and class constants
   * "unsoundly".  I.e., it is visible to the user that we may not
   * invoke autoload at places where we would have without this
   * optimization.
   */
  bool HardConstProp = true;

  /*
   * Whether or not to assume that VerifyParamType instructions must
   * throw if the parameter does not match the associated type
   * constraint.
   */
  bool HardTypeHints = true;
} options;

//////////////////////////////////////////////////////////////////////

}}

#endif
