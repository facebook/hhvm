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

/*
 * This is the Verifier's public interface.
 */

#pragma once

#include "hphp/runtime/vm/verifier/util.h"

namespace HPHP {

struct UnitEmitter;
struct FuncEmitter;

namespace Verifier {

enum ErrorMode {
  kStderr,
  kVerbose,
  kThrow
};

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
 * -- PreClasses
 *
 * Not Checked:
 * -- SourceLoc
 * -- Metadata
 */
bool checkUnit(const UnitEmitter*, ErrorMode mode = kStderr);

/**
 * Checker for one Func.  Rules from doc/bytecode.specification:
 * Checked:
 * 1.  Depth of eval stack must be same for any control-flow path.
 *     flavor descriptors for every stack element are same for any path.
 * 2.  Stack underflow & overflow not allowed.
 * 3.  Empty stack at try-region starts (but not ends).
 * 4.  |stack| == 1 before Ret*.
 * 5.  The body must end with a terminal.
 * 6.  State of each iterator variable known everywhere.
 * 7.  initialized state of iterators correct for Iter* instructions.
 * 8.  Asserts not separated from following instruction by control flow
 * 9.  Member instruction sequences are consistent and continuous
 * -- All region and branch offsets must refer to valid instruction starts.
 * -- every string table index in-bounds
 * -- every array table index in-bounds
 * -- Local variable ids must be < Func.numLocals
 * -- iter variable ids must be < Func.numIterators
 * -- init-state of every iterator must be known everywhere
 *
 * Not Checked:
 * 1.  empty stack at try-region ends (but starts are checked).  And what
 *     does this mean? -- linear-end or flow-end?
 * 2.  eval stack must be empty in blocks that come before all preds.
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
bool checkFunc(const FuncEmitter*,
               StringToStringTMap& createCls,
               ErrorMode mode = kStderr);

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
bool checkNativeFunc(const FuncEmitter*, ErrorMode mode = kStderr);

}} // HPHP::Verifier
