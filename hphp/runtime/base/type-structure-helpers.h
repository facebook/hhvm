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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/req-vector.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/runtime/vm/bytecode.h"


namespace HPHP {

/*
 * Checks whether the given cell is an instance of the class referred by
 * given named type.
 */
bool tvInstanceOf(const TypedValue* tv, const NamedType* ne);

/*
 * Checks whether the given cell is an instance of the given class.
 */
bool tvInstanceOf(const TypedValue* tv, const Class* cls);

/*
 * Returns true is all the generics on the type type structure are
 * wildcards. If the type structure does not contain any generics, then
 * returns true.
 */
bool isTSAllWildcards(const ArrayData* ts);

/*
 * Returns whether the typed value matches the type structure. This function
 * checks whether the reified generics match as well.
 * Sets the warn flag if the type parameter is denoted as soft either through
 * an annotation at the declaration site or by soft type hint at the generic
 * level
 */
bool verifyReifiedLocalType(
  tv_rval param,
  const ArrayData* type,
  const Class* ctx,
  const Func* func,
  bool isTypeVar,
  bool& warn
);

/*
 * Resolves the given Array as a type structure.
 *
 * Raises an error if the type structure contains a trait, typevar or a function
 * type.
 * If suppress is not set, will raise an error if the type structure cannot be
 * resolved.
 * Otherwise, if suppress and RuntimeOption::EvalIsExprEnableUnresolvedWarning
 * are set, will raise a warning instead.
 *
 * Because the type structure might contain unresolved nested structures, we
 * must manually supply the called class and the declaring class so that `this`,
 *`self`, and `parent` references are resolved correctly and error messages
 * are displayed correctly.
 */
template <bool IsOrAsOp>
Array resolveAndVerifyTypeStructure(
  const Array& ts,
  const Class* declaringCls,
  const Class* calledCls,
  const req::vector<Array>& tsList,
  bool suppress
);

/*
 * Raises an error if the type hint for is/as expression contains an invalid
 * type such as callables, erased type variables and trait type hints
 * If dryrun is set, it does not actually raise the error
 * If allowWildcard is set, then it does not error on wildcards
 */
bool errorOnIsAsExpressionInvalidTypes(const Array& ts, bool dryrun,
                                       bool allowWildcard = false);

/**
 * Returns whether the type structure may not be able to be resolved statically,
 * i.e. if it contains `this` references.
 */
bool typeStructureCouldBeNonStatic(const ArrayData* ts);

/*
 * Checks whether the type of the given cell matches the type structure.
 * Expects a resolved type structure.
 */
bool checkTypeStructureMatchesTV(const Array& ts, TypedValue c1);

/*
 * In addition to regular checkTypeStructureMatchesTV, also populates the
 * error paths
 */
bool checkTypeStructureMatchesTV(
  const Array& ts,
  TypedValue c1,
  std::string& givenType,
  std::string& expectedType,
  std::string& errorKey
);

/*
 * In addition to regular checkTypeStructureMatchesTV, also sets the warn flag
 * if the type parameter is denoted as soft either through an annotation at the
 * declaration site or by soft type hint at the generic level
 */
bool checkTypeStructureMatchesTV(const Array& ts, TypedValue c1, bool& warn);

/*
 * Throws user catchable exception that tells the user what the given type is,
 * what the expected type is and which key it failed at, if applicable
 */
[[noreturn]] void throwTypeStructureDoesNotMatchTVException(
  std::string& givenType,
  std::string& expectedType,
  std::string& errorKey,
  bool throwFatal
);

/*
 * Checks to see whether the input type structure contains a T_Unresolved
 */
bool doesTypeStructureContainTUnresolved(const ArrayData* ts);

}
