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

#include <string>
#include <utility>

#include "hphp/hhbbc/hhbbc.h"

namespace HPHP {

namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * Publically-settable options that control compilation.
 */
struct Options {
  /*
   * When debugging, it can be useful to ask for certain functions to be traced
   * at a higher level than the rest of the program.
   */
  MethodMap TraceFunctions;

  /*
   * When debugging, it can be useful to ask for a list of functions
   * that use particular bytecodes.
   */
  OpcodeSet TraceBytecodes;

  /*
   * If non-empty, dump jemalloc memory profiles at key points during
   * the build, using this as a prefix.
   */
  std::string profileMemory;

  //////////////////////////////////////////////////////////////////////

  /*
   * Flags for various limits on when to perform widening operations.
   * See analyze.cpp for details.
   */
  uint32_t analyzeFuncWideningLimit = 8;
  uint32_t analyzeClassWideningLimit = 6;

  /*
   * When to stop refining return types.
   *
   * This needs to be limited because types can walk downwards in our
   * type lattice indefinitely.  The index never contains incorrect
   * return types, since the return types only shrink, which means we
   * can just stop refining a return type whenever we want to without
   * causing problems.
   *
   * For an example of where this can occur, imagine the analysis of the
   * following function:
   *
   *    function foo() { return array('x' => foo()); }
   *
   * Each time we visit `foo', we'll discover a slightly smaller return
   * type, in a downward-moving sequence that would never terminate:
   *
   *   InitCell, CArrN(x:InitCell), CArrN(x:CArrN(x:InitCell)), ...
   */
  uint32_t returnTypeRefineLimit = 8;

  /*
   * Limit public static property refinement for the same reason.
   */
  uint32_t publicSPropRefineLimit = 8;

  /*
   * Whether to produce extended stats information.  (Takes extra
   * time.)
   */
  bool extendedStats = false;

  //////////////////////////////////////////////////////////////////////

  /*
   * If true, all optimizations are disabled, and analysis isn't even
   * performed.
   */
  bool NoOptimizations = false;

  /*
   * If true, analyze calls to functions in a context-sensitive way.
   *
   * Disabled by default because its slow, with very little gain.
   */
  bool ContextSensitiveInterp = false;

  /*
   * If true, completely remove jumps to blocks that are inferred to be dead.
   * When false, dead blocks are replaced with Fatal bytecodes.
   */
  bool RemoveDeadBlocks = true;

  /*
   * Whether to propagate constant values by replacing instructions which are
   * known to always produce a constant with instructions that produce that
   * constant.
   */
  bool ConstantProp = true;

  /*
   * Whether we should evaluate side-effect free builtins at compile time when
   * they have compile-time constant arguments.
   */
  bool ConstantFoldBuiltins = true;

  /*
   * Whether to perform local or global dead code elimination.  This removes
   * unnecessary instructions within a single block, or across blocks,
   * respectively.
   */
  bool LocalDCE = true;
  bool GlobalDCE = true;

  /*
   * Whether to remove completely unused local names.  This requires
   * GlobalDCE.
   */
  bool RemoveUnusedLocalNames = true;

  /*
   * Whether to compact local slot usage by having non conflicting locals share
   * a local slot.  This requires GlobalDCE.
   */
  bool CompactLocalSlots = true;

  /*
   * If true, insert opcodes that assert inferred types, so we can assume them
   * at runtime.
   */
  bool InsertAssertions = true;
  bool InsertStackAssertions = true;

  /*
   * If true, try to filter asserts out that are "obvious" (this is a code size
   * optimization).  It can be useful to turn this option off for debugging.
   *
   * Has no effect if !InsertStackAssertions.
   */
  bool FilterAssertions = true;

  /*
   * Whether to replace bytecode with less expensive bytecodes when we can.
   * E.g. InstanceOf -> InstanceOfD or FCallFunc -> FCallFuncD.
   */
  bool StrengthReduce = true;

  /*
   * Whether to enable 'FuncFamily' method resolution.
   *
   * This allows possible overrides of a method to be resolved as a
   * set of candidates when we aren't sure which one it would be.
   */
  bool FuncFamilies = true;

  /*
   * Should we do an extra whole-program pass to try to determine the types of
   * public static properties.  This will not yield any useful information for
   * programs that contain any sets to static properties with both a dynamic
   * property name and an unknown class type.
   */
  bool AnalyzePublicStatics = true;

  //////////////////////////////////////////////////////////////////////
  // Flags below this line perform optimizations that intentionally
  // may have user-visible changes to program behavior.
  //////////////////////////////////////////////////////////////////////

  /*
   * If true, we'll try to infer the types of declared private class
   * properties.
   *
   * This is in the can-potentially-change-program-behavior section
   * because if you unserialize specially-constructed strings you
   * could create instances with private properties that don't follow
   * the inferred types.  HHVM tracks the types that were inferred,
   * and if an unserialize happens that would violate what we've
   * inferred, we'll raise a notice and unserialize() returns false.
   */
  bool HardPrivatePropInference = true;

  /*
   * Whether to flatten trait methods and properties into the classes
   * that use them.
   */
  bool FlattenTraits = true;

  /*
   * The filepath where to save the stats file.  If the path is empty, then we
   * save the stats file to a temporary file.
   */
  std::string stats_file;

  /*
   * Run a test of HHBBC memory compression (e.g. bytecode compression).
   */
  bool TestCompression;
};
extern Options options;

}}
