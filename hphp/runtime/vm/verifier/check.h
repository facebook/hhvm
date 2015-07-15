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

/*
 * This is the Verifier's public interface.
 */

#ifndef incl_HPHP_VM_VERIFIER_CHECK_H_
#define incl_HPHP_VM_VERIFIER_CHECK_H_

namespace HPHP {

class Unit;
class Func;

namespace Verifier {

/**
 * Check one whole unit, including its internal string, array, sourceLoc,
 * preClass, and func tables.
 *
 * Checked:
 * -- string table may not contain null pointers, but strings can contain
 *    NUL (0) chars.
 * -- array table may not contain null pointers
 * -- every byte of code must be in exactly one Func's range.
 * -- must have exactly 1 pseudo-main
 * -- checkFunc for each function in the unit
 *
 * Not Checked:
 * -- SourceLocs
 * -- PreClasses
 * -- Metadata
 */
bool checkUnit(const Unit*, bool verbose = false);

/**
 * Checker for one Func.  Rules from doc/bytecode.specification:
 * Checked:
 * 1.  Depth of eval stack must be same for any control-flow path.
 *     flavor descriptors for every stack element are same for any path.
 * 2.  Stack underflow & overflow not allowed.  INS_1 and INS_2 stack
 *     refs must be in-bounds.
 * 3.  Empty stack at try-region starts (but not ends).
 * 5.  |stack| == 1 before Ret*, == 0 before Unwind.
 * 6.  no jumps between sections, where section is main body or any funclet.
 * 7.  each section must end with a terminal;  main body cannot contain Unwind;
 *     Funclets may not contain Ret*.
 * 10. FPI depth same for all ctrl paths. every path must have N FPass's
 *     and params must be passed in forward order.
 * 11. stack depth @FPush == depth @FCall.  No instr can pop past depth of
 *     FPush.
 * 12. State of each iterator variable known everywhere.
 * 13. initialized state of iterators correct for Iter* instructions.
 * -- All region and branch offsets must refer to valid instruction starts.
 * -- Every FPI region is wholly contained in one body/funclet section.
 * -- every string table index in-bounds
 * -- every array table index in-bounds
 * -- Local variable ids must be < Func.numLocals
 * -- iter variable ids must be < Func.numIterators
 * -- FPass* parameter ids must be < FPush*'s <num params>
 * -- FCall <num params> == FPush* <num params>
 * -- init-state of every iterator must be known everywhere
 *
 * Not Checked:
 * 3.  empty stack at try-region ends (but starts are checked).  And what
 *     does this mean? -- linear-end or flow-end?
 * 4.  eval stack must be empty in blocks that come before all preds.
 * 8.  each fpi starts with FPush* and ends with FCall; each FPush must be
 *     the first instr in exactly 1 fpi region; FPass* never outside FPI.
 * 9.  no back-jumps in FPI; no forward jumps out of FPI; no jumps into
 *     FPI from outside; no terminals inside FPI region.
 * -- FuncVar entries must refer to valid local ids; no local can have
 *    2+ names.
 * -- FuncStaticVar not checked
 * -- FuncUserAttribute table not checked
 * -- control-flow from DV entry points isn't checked -- we dont require
 *    that entry K fall through to K+1, or whatever.  We either should
 *    check or make the spec say what uninitialized optional params
 *    are set to (Uninit null like uninitialized locals?).
 * -- Function attributes (static, abstract, etc) must all make sense,
 *    certian attributes are mutually exclusive, others aren't, some
 *    imply bytecode restrictions.  (access This from static? etc).
 */
bool checkFunc(const Func*, bool verbose = false);

/**
 * Checker for HNI native function signatures. Verifies that argument types
 * declared for C++ native implementations match the corresponding PHP types
 * in the systemlib unit.
 *
 * Checked:
 * -- Primitive argument types are correct: int64_t, bool, and double
 * -- Complex arguments use correct req::ptr wrapper: String, Object,
 *    Resource, and Array
 * -- Mixed arguments are taken as Variant
 * -- By-reference arguments are taken as VRefParam
 * -- Return types are also correct
 * -- Functions declared as ActRec take an ActRec* as their sole argument
 * -- Functions with a variadic parameter take an Array as the final argument,
 *    and PHP 5 compatibility functions are registred appropriately.
 * -- Methods take an ObjectData* as their first argument
 * -- Static methods take a const Class* as their first argument
 */
bool checkNativeFunc(const Func*, bool verbose = false);

}} // HPHP::Verifier

#endif // incl_HPHP_VM_VERIFIER_CHECK_UNIT_H_
