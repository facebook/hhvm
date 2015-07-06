/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_HHBBC_H_
#define incl_HPHP_HHBBC_H_

#include <vector>
#include <memory>
#include <string>
#include <utility>
#include <map>
#include <set>

#include "hphp/util/functional.h"

#include "hphp/runtime/base/repo-auth-type-array.h"

namespace HPHP { struct UnitEmitter; }
namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * This is the public API to this subsystem.
 */

//////////////////////////////////////////////////////////////////////

using MethodMap = std::map<
  std::string,
  std::set<std::string,stdltistr>,
  stdltistr
>;

// Create a method map for the options structure from a SinglePassReadableRange
// containing a list of Class::methodName strings.
template<class SinglePassReadableRange>
MethodMap make_method_map(SinglePassReadableRange&);

//////////////////////////////////////////////////////////////////////

/*
 * Publically-settable options that control compilation.
 */
struct Options {
  /*
   * Functions that we should assume may be used with fb_intercept.  Functions
   * that aren't named in this list may be optimized with the assumption they
   * aren't intercepted, in whole_program mode.
   *
   * If AllFuncsInterceptable, it's as if this list contains every function in
   * the program.
   */
  MethodMap InterceptableFunctions;
  bool AllFuncsInterceptable = false;

  /*
   * When debugging, it can be useful to ask for certain functions to be traced
   * at a higher level than the rest of the program.
   */
  MethodMap TraceFunctions;

  //////////////////////////////////////////////////////////////////////

  /*
   * Flags for various limits on when to perform widening operations.
   * See analyze.cpp for details.
   */
  uint32_t analyzeFuncWideningLimit = 50;
  uint32_t analyzeClassWideningLimit = 20;

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
  uint32_t returnTypeRefineLimit = 15;

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
   * Note, this is disabled by default because of the need to have an
   * intersection operation in the type system to maintain index
   * invariants---it doesn't quite work yet.  See comments in index.cpp.
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
   * Whether to remove completely unused local variables.  This requires
   * GlobalDCE.
   */
  bool RemoveUnusedLocals = true;

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
   * E.g. InstanceOf -> InstanceOfD or FPushFunc -> FPushFuncD.
   */
  bool StrengthReduce = true;

  /*
   * Whether to turn on peephole optimizations (e.g., Concat, ..., Concat ->
   * ..., ConcatN).
   */
  bool Peephole = true;

  /*
   * Whether to enable 'FuncFamily' method resolution.
   *
   * This allows possible overrides of a method to be resolved as a
   * set of candidates when we aren't sure which one it would be.
   */
  bool FuncFamilies = true;

  /*
   * Whether or not hhbbc should attempt to do anything intelligent to
   * pseudomains.
   */
  bool AnalyzePseudomains = true;

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
   * If true, we'll propagate global defined constants, class constants, and
   * constant static class properties "unsoundly".  I.e., it is visible to the
   * user that we may not invoke autoload at places where we would have without
   * this optimization.
   */
  bool HardConstProp = true;

  /*
   * Whether or not to assume that VerifyParamType instructions must
   * throw if the parameter does not match the associated type
   * constraint.
   *
   * This changes program behavior because parameter type hint
   * validation is normally a recoverable fatal.  When this option is
   * on, hhvm will fatal if the error handler tries to recover in this
   * situation.
   */
  bool HardTypeHints = true;

  /*
   * Whether or not to assume that VerifyRetType* instructions must
   * throw if the parameter does not match the associated type
   * constraint.
   *
   * This changes program behavior because return type hint validation
   * is normally a recoverable fatal.  When this option is on, hhvm will
   * fatal if the error handler tries to recover in this situation.
   */
  bool HardReturnTypeHints = false;

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
   * If true, we'll assume that dynamic function calls (like '$f()') do not
   * have effects on unknown locals (i.e. are not extract / compact /...).
   * See, e.g. __SystemLib\\extract vs extract.
   */
  bool DisallowDynamicVarEnvFuncs = true;

  /*
   * The filepath where to save the stats file.  If the path is empty, then we
   * save the stats file to a temporary file.
   */
  std::string stats_file;
};
extern Options options;

//////////////////////////////////////////////////////////////////////

/*
 * Perform whole-program optimization on a set of UnitEmitters.
 *
 * Currently this process relies on some information from HPHPc.  It
 * expects AttrUnique/AttrPersistent have already been set up
 * correctly (but won't be wrong if they aren't set up at all), and
 * expects traits are already flattened (it might be wrong if they
 * aren't).
 */
std::pair<
  std::vector<std::unique_ptr<UnitEmitter>>,
  std::unique_ptr<ArrayTypeTable::Builder>
>
whole_program(std::vector<std::unique_ptr<UnitEmitter>>);

//////////////////////////////////////////////////////////////////////

/*
 * Main entry point when the program should behave like hhbbc.
 */
int main(int argc, char** argv);

//////////////////////////////////////////////////////////////////////

}}

#include "hphp/hhbbc/hhbbc-inl.h"

#endif
