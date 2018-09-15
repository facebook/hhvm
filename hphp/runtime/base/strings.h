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

#ifndef incl_HPHP_STRINGS_H_
#define incl_HPHP_STRINGS_H_

namespace HPHP {
namespace Strings {

const char* const FATAL_NULL_THIS = "$this is null";
const char* const WARN_NULL_THIS = "Undefined variable: this";
const char* const ASSIGN_THIS_ERROR = "Cannot re-assign $this";
const char* const FUNCTION_ALREADY_DEFINED = "Function already defined: %s";
const char* const CONSTANT_ALREADY_DEFINED = "Constant %s already defined";
const char* const CONSTANTS_MUST_BE_SCALAR =
  "Constants may only evaluate to scalar values or arrays";
const char* const CONSTANTS_CASE_SENSITIVE =
  "Case insensitive constant names are not supported in HipHop";
const char* const MODULO_BY_ZERO = "Modulo by zero";
const char* const DIVISION_BY_ZERO =
  "Division by zero";
const char* const NEGATIVE_SHIFT = "Bit shift by negative number";
const char* const UNDEFINED_CONSTANT =
  "Use of undefined constant %s - assumed '%s'";
const char* const UNDEFINED_VARIABLE = "Undefined variable: %s";
const char* const UNKNOWN_CLASS = "Class undefined: %s";
const char* const CANT_ACCESS_SELF =
  "Cannot access self:: when no class scope is active";
const char* const CANT_ACCESS_PARENT_WHEN_NO_CLASS =
  "Cannot access parent:: when no class scope is active";
const char* const CANT_ACCESS_PARENT_WHEN_NO_PARENT =
  "Cannot access parent:: when current class scope has no parent";
const char* const CANT_ACCESS_STATIC =
  "Cannot access static:: when no class scope is active";
const char* const UNDEFINED_INDEX =
  "Undefined index: %s";
const char* const CANNOT_USE_SCALAR_AS_ARRAY =
  "Cannot use a scalar value as an array";
const char* const CREATING_DEFAULT_OBJECT =
  "Creating default object from empty value";
const char* const SET_PROP_NON_OBJECT =
  "Setting a property on a non-object";
const char* const NULLSAFE_PROP_WRITE_ERROR =
  "?-> is not allowed in write context";
const char* const NULLSAFE_THIS_BASE_ERROR = "?-> is not allowed with $this";
const char* const FUNCTION_NAME_MUST_BE_STRING =
  "Function name must be a string";
const char* const METHOD_NAME_MUST_BE_STRING =
  "Method name must be a string";
const char* const MISSING_ARGUMENT_EXCEPTION =
  "Too few arguments to function %s(), %d passed and %s %d expected";
const char* const MISSING_ARGUMENT =
  "%s() expects %s 1 parameter, %d given";
const char* const MISSING_ARGUMENTS =
  "%s() expects %s %d parameters, %d given";
const char* const CANT_UNSET_STRING =
  "Cannot unset string offsets";
const char* const OP_NOT_SUPPORTED_STRING =
  "Operator not supported for strings";
const char* const ASYNC_WITHOUT_BODY =
  "Cannot declare %s method %s::%s() async; async is only meaningful"
  " when it modifies a method body";
const char* const PICK_ACCESS_MODIFIER =
  "Multiple access type modifiers are not allowed";
const char* const TRAITS_UNKNOWN_TRAIT =
  "Unknown trait '%s'";
const char* const TRAITS_UNKNOWN_TRAIT_METHOD =
  "Unknown trait method '%s'";
const char* const METHOD_IN_MULTIPLE_TRAITS =
  "Method '%s' declared in multiple traits";
const char* const TRAIT_REQ_EXTENDS =
  "Class '%s' required to extend class '%s'"
  " by trait '%s'";
const char* const TRAIT_REQ_IMPLEMENTS =
  "Class '%s' required to implement interface '%s'"
  " by trait '%s'";
const char* const TRAIT_BAD_REQ_EXTENDS =
  "Trait '%s' requires extension of '%s', but %s "
  "is not an extendable class";
const char* const TRAIT_BAD_REQ_IMPLEMENTS =
  "Trait '%s' requires implementations of '%s', but %s "
  "is not an interface";
const char* const INCONSISTENT_INSTEADOF =
  "Inconsistent insteadof definition. The method %s is to be used from %s, "
  "but %s is also on the exclude list";
const char* const MULTIPLY_EXCLUDED =
  "Failed to evaluate a trait precedence (%s). Method of trait %s was defined "
  "to be excluded multiple times";
const char* const REDECLARE_BUILTIN = "Cannot redeclare %s()";
const char* const DISALLOWED_DYNCALL = "%s should not be called dynamically";
const char* const HACKARR_COMPAT_ARR_MIXEDCMP =
  "Comparing array with non-array";
const char* const HACKARR_COMPAT_VARR_IS_ARR = "is_array() called on varray";
const char* const HACKARR_COMPAT_DARR_IS_ARR = "is_array() called on darray";
const char* const HACKARR_COMPAT_VEC_IS_ARR = "is_array() called on vec";
const char* const HACKARR_COMPAT_DICT_IS_ARR = "is_array() called on dict";
const char* const HACKARR_COMPAT_KEYSET_IS_ARR = "is_array() called on keyset";
const char* const HACKARR_COMPAT_VARR_IS_VEC = "is_vec() called on varray";
const char* const HACKARR_COMPAT_VEC_IS_VARR = "is_varray() called on vec";
const char* const HACKARR_COMPAT_DARR_IS_DICT = "is_dict() called on darray";
const char* const HACKARR_COMPAT_DICT_IS_DARR = "is_darray() called on dict";
const char* const HACKARR_COMPAT_TUPLE_IS_DARR =
  "is/as operator used with darray and tuple";
const char* const HACKARR_COMPAT_SHAPE_IS_VARR =
  "is/as operator used with varray and shape";
const char* const FUNCTION_CALLED_DYNAMICALLY = "'%s' called dynamically";

} // namespace Strings
} // namespace HPHP

#endif // incl_HPHP_STRINGS_H_
