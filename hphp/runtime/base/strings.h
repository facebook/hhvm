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

auto constexpr FATAL_NULL_THIS = "$this is null";
auto constexpr WARN_NULL_THIS = "Undefined variable: this";
auto constexpr ASSIGN_THIS_ERROR = "Cannot re-assign $this";
auto constexpr FUNCTION_ALREADY_DEFINED = "Function already defined: %s";
auto constexpr CONSTANT_ALREADY_DEFINED = "Constant %s already defined";
auto constexpr CONSTANTS_MUST_BE_SCALAR =
  "Constants may only evaluate to scalar values or arrays";
auto constexpr CONSTANTS_CASE_SENSITIVE =
  "Case insensitive constant names are not supported in HipHop";
auto constexpr MODULO_BY_ZERO = "Modulo by zero";
auto constexpr DIVISION_BY_ZERO =
  "Division by zero";
auto constexpr NEGATIVE_SHIFT = "Bit shift by negative number";
auto constexpr UNDEFINED_VARIABLE = "Undefined variable: %s";
auto constexpr UNKNOWN_CLASS = "Class undefined: %s";
auto constexpr UNKNOWN_RECORD = "Record undefined: %s";
auto constexpr CANT_ACCESS_SELF =
  "Cannot access self:: when no class scope is active";
auto constexpr CANT_ACCESS_PARENT_WHEN_NO_CLASS =
  "Cannot access parent:: when no class scope is active";
auto constexpr CANT_ACCESS_PARENT_WHEN_NO_PARENT =
  "Cannot access parent:: when current class scope has no parent";
auto constexpr CANT_ACCESS_STATIC =
  "Cannot access static:: when no class scope is active";
auto constexpr THIS_OUTSIDE_CLASS =
  "Cannot use 'this' outside of a class";
auto constexpr UNDEFINED_INDEX =
  "Undefined index: %s";
auto constexpr CANNOT_USE_SCALAR_AS_ARRAY =
  "Cannot use a scalar value as an array";
auto constexpr CREATING_DEFAULT_OBJECT =
  "Creating default object from empty value";
auto constexpr SET_PROP_NON_OBJECT =
  "Setting a property on a non-object";
auto constexpr NULLSAFE_PROP_WRITE_ERROR =
  "?-> is not allowed in write context";
auto constexpr NULLSAFE_THIS_BASE_ERROR = "?-> is not allowed with $this";
auto constexpr FUNCTION_NAME_MUST_BE_STRING =
  "Function name must be a string";
auto constexpr METHOD_NAME_MUST_BE_STRING =
  "Method name must be a string";
auto constexpr CANT_UNSET_STRING =
  "Cannot unset string offsets";
auto constexpr OP_NOT_SUPPORTED_STRING =
  "Operator not supported for strings";
auto constexpr OP_NOT_SUPPORTED_FUNC =
  "Operator not supported for funcs";
auto constexpr OP_NOT_SUPPORTED_CLASS =
  "Operator not supported for classes";
auto constexpr ASYNC_WITHOUT_BODY =
  "Cannot declare %s method %s::%s() async; async is only meaningful"
  " when it modifies a method body";
auto constexpr PICK_ACCESS_MODIFIER =
  "Multiple access type modifiers are not allowed";
auto constexpr TRAITS_UNKNOWN_TRAIT =
  "Unknown trait '%s'";
auto constexpr TRAITS_UNKNOWN_TRAIT_METHOD =
  "Unknown trait method '%s'";
auto constexpr METHOD_IN_MULTIPLE_TRAITS =
  "Method '%s' declared in multiple traits";
auto constexpr TRAIT_REDECLARED_METHOD_INCONSISTENT_ATTRIBUTES =
  "Redeclaration of trait method '%s::%s' is inconsistent about '%s'";
auto constexpr TRAIT_REDECLARED_FINAL_METHOD=
  "Redeclaration of final trait method '%s::%s' must also be final";
auto constexpr TRAIT_REQ_EXTENDS =
  "Class '%s' required to extend class '%s'"
  " by trait '%s'";
auto constexpr TRAIT_REQ_IMPLEMENTS =
  "Class '%s' required to implement interface '%s'"
  " by trait '%s'";
auto constexpr TRAIT_BAD_REQ_EXTENDS =
  "Trait '%s' requires extension of '%s', but %s "
  "is not an extendable class";
auto constexpr TRAIT_BAD_REQ_IMPLEMENTS =
  "Trait '%s' requires implementations of '%s', but %s "
  "is not an interface";
auto constexpr INCONSISTENT_INSTEADOF =
  "Inconsistent insteadof definition. The method %s is to be used from %s, "
  "but %s is also on the exclude list";
auto constexpr MULTIPLY_EXCLUDED =
  "Failed to evaluate a trait precedence (%s). Method of trait %s was defined "
  "to be excluded multiple times";
auto constexpr REDECLARE_BUILTIN = "Cannot redeclare %s()";
auto constexpr DISALLOWED_DYNCALL = "%s should not be called dynamically";
auto constexpr HACKARR_COMPAT_ARR_MIXEDCMP =
  "Comparing array with non-array";
auto constexpr HACKARR_COMPAT_VARR_IS_ARR = "is_array() called on varray";
auto constexpr HACKARR_COMPAT_DARR_IS_ARR = "is_array() called on darray";
auto constexpr HACKARR_COMPAT_VEC_IS_ARR = "is_array() called on vec";
auto constexpr HACKARR_COMPAT_DICT_IS_ARR = "is_array() called on dict";
auto constexpr HACKARR_COMPAT_KEYSET_IS_ARR = "is_array() called on keyset";
auto constexpr HACKARR_COMPAT_VARR_IS_VEC = "is_vec() called on varray";
auto constexpr HACKARR_COMPAT_VEC_IS_VARR = "is_varray() called on vec";
auto constexpr HACKARR_COMPAT_DARR_IS_DICT = "is_dict() called on darray";
auto constexpr HACKARR_COMPAT_DICT_IS_DARR = "is_darray() called on dict";
auto constexpr FUNCTION_CALLED_DYNAMICALLY = "'%s' called dynamically";
auto constexpr CLASS_CONSTRUCTED_DYNAMICALLY = "'%s' constructed dynamically";
auto constexpr REIFIED_GENERICS_NOT_GIVEN =
  "Cannot call the reified function '%s' without the reified generics";
auto constexpr NEW_STATIC_ON_REIFIED_CLASS =
  "Cannot call new static since class %s has reified generics";
auto constexpr RECORD_NOT_SUPPORTED =
  "Records are not supported here";

auto constexpr CLSMETH_COMPAT_IS_ARR = "is_array() called on clsmeth";
auto constexpr CLSMETH_COMPAT_IS_VEC = "is_vec() called on clsmeth";
auto constexpr CLSMETH_COMPAT_IS_VARR = "is_varray() called on clsmeth";
auto constexpr CLSMETH_COMPAT_IS_ANY_ARR = "is_any_array() called on clsmeth";

} // namespace Strings
} // namespace HPHP

#endif // incl_HPHP_STRINGS_H_
