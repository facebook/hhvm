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

constexpr char FATAL_NULL_THIS[] = "$this is null";
constexpr char WARN_NULL_THIS[] = "Undefined variable: this";
constexpr char ASSIGN_THIS_ERROR[] = "Cannot re-assign $this";
constexpr char FUNCTION_ALREADY_DEFINED[] = "Function already defined: %s";
constexpr char CONSTANT_ALREADY_DEFINED[] = "Constant %s already defined";
constexpr char CONSTANTS_MUST_BE_SCALAR[] =
  "Constants may only evaluate to scalar values or arrays";
constexpr char CONSTANTS_CASE_SENSITIVE[] =
  "Case insensitive constant names are not supported in HipHop";
constexpr char MODULO_BY_ZERO[] = "Modulo by zero";
constexpr char DIVISION_BY_ZERO[] =
  "Division by zero";
constexpr char NEGATIVE_SHIFT[] = "Bit shift by negative number";
constexpr char UNDEFINED_VARIABLE[] = "Undefined variable: %s";
constexpr char UNKNOWN_CLASS[] = "Class undefined: %s";
constexpr char UNKNOWN_RECORD[] = "Record undefined: %s";
constexpr char CANT_ACCESS_SELF[] =
  "Cannot access self:: when no class scope is active";
constexpr char CANT_ACCESS_PARENT_WHEN_NO_CLASS[] =
  "Cannot access parent:: when no class scope is active";
constexpr char CANT_ACCESS_PARENT_WHEN_NO_PARENT[] =
  "Cannot access parent:: when current class scope has no parent";
constexpr char CANT_ACCESS_STATIC[] =
  "Cannot access static:: when no class scope is active";
constexpr char THIS_OUTSIDE_CLASS[] =
  "Cannot use 'this' outside of a class";
constexpr char CANNOT_USE_SCALAR_AS_ARRAY[] =
  "Cannot use a scalar value as an array";
constexpr char CREATING_DEFAULT_OBJECT[] =
  "Creating default object from empty value";
constexpr char SET_PROP_NON_OBJECT[] =
  "Setting a property on a non-object";
constexpr char NULLSAFE_PROP_WRITE_ERROR[] =
  "?-> is not allowed in write context";
constexpr char NULLSAFE_THIS_BASE_ERROR[] = "?-> is not allowed with $this";
constexpr char FUNCTION_NAME_MUST_BE_STRING[] =
  "Function name must be a string";
constexpr char METHOD_NAME_MUST_BE_STRING[] =
  "Method name must be a string";
constexpr char CANT_UNSET_STRING[] =
  "Cannot unset string offsets";
constexpr char CANT_UNSET_RECORD[] =
  "Cannot unset record fields";
constexpr char OP_NOT_SUPPORTED_STRING[] =
  "Operator not supported for strings";
constexpr char OP_NOT_SUPPORTED_FUNC[] =
  "Operator not supported for funcs";
constexpr char OP_NOT_SUPPORTED_RECORD[] =
  "Operator not supported for records";
constexpr char OP_NOT_SUPPORTED_CLASS[] =
  "Operator not supported for classes";
constexpr char ASYNC_WITHOUT_BODY[] =
  "Cannot declare %s method %s::%s() async; async is only meaningful"
  " when it modifies a method body";
constexpr char PICK_ACCESS_MODIFIER[] =
  "Multiple access type modifiers are not allowed";
constexpr char TRAITS_UNKNOWN_TRAIT[] =
  "Unknown trait '%s'";
constexpr char TRAITS_UNKNOWN_TRAIT_METHOD[] =
  "Unknown trait method '%s'";
constexpr char METHOD_IN_MULTIPLE_TRAITS[] =
  "Method '%s' declared in multiple traits (%s)";
constexpr char TRAIT_REDECLARED_METHOD_INCONSISTENT_ATTRIBUTES[] =
  "Redeclaration of trait method '%s::%s' is inconsistent about '%s'";
auto constexpr TRAIT_REDECLARED_FINAL_METHOD=
  "Redeclaration of final trait method '%s::%s' must also be final";
constexpr char TRAIT_REQ_EXTENDS[] =
  "Class '%s' required to extend class '%s'"
  " by trait '%s'";
constexpr char TRAIT_REQ_IMPLEMENTS[] =
  "Class '%s' required to implement interface '%s'"
  " by trait '%s'";
constexpr char TRAIT_BAD_REQ_EXTENDS[] =
  "Trait '%s' requires extension of '%s', but %s "
  "is not an extendable class";
constexpr char TRAIT_BAD_REQ_IMPLEMENTS[] =
  "Trait '%s' requires implementations of '%s', but %s "
  "is not an interface";
constexpr char INCONSISTENT_INSTEADOF[] =
  "Inconsistent insteadof definition. The method %s is to be used from %s, "
  "but %s is also on the exclude list";
constexpr char MULTIPLY_EXCLUDED[] =
  "Failed to evaluate a trait precedence (%s). Method of trait %s was defined "
  "to be excluded multiple times";
constexpr char REDECLARE_BUILTIN[] = "Cannot redeclare %s()";
constexpr char DISALLOWED_DYNCALL[] = "%s should not be called dynamically";
constexpr char HACKARR_COMPAT_ARR_HACK_ARR_CMP[] =
  "Comparing PHP array with Hack array";
constexpr char HACKARR_COMPAT_ARR_NON_ARR_CMP[] =
  "Comparing PHP array with non any-array";
constexpr char HACKARR_COMPAT_VARR_IS_ARR[] = "is_array() called on varray";
constexpr char HACKARR_COMPAT_DARR_IS_ARR[] = "is_array() called on darray";
constexpr char HACKARR_COMPAT_VEC_IS_ARR[] = "is_array() called on vec";
constexpr char HACKARR_COMPAT_DICT_IS_ARR[] = "is_array() called on dict";
constexpr char HACKARR_COMPAT_KEYSET_IS_ARR[] = "is_array() called on keyset";
constexpr char HACKARR_COMPAT_VARR_IS_VEC[] = "is_vec() called on varray";
constexpr char HACKARR_COMPAT_VEC_IS_VARR[] = "is_varray() called on vec";
constexpr char HACKARR_COMPAT_DARR_IS_DICT[] = "is_dict() called on darray";
constexpr char HACKARR_COMPAT_DICT_IS_DARR[] = "is_darray() called on dict";
constexpr char FUNCTION_CALLED_DYNAMICALLY_WITHOUT_ATTRIBUTE[] =
  "'%s' called dynamically but is not __DynamicallyCallable";
constexpr char FUNCTION_CALLED_DYNAMICALLY_WITH_ATTRIBUTE[] =
  "'%s' called dynamically without using a function pointer";
constexpr char CLASS_CONSTRUCTED_DYNAMICALLY[] = "'%s' constructed dynamically";
constexpr char NEW_STATIC_ON_REIFIED_CLASS[] =
  "Cannot call new static since class %s has reified generics";
constexpr char RECORD_NOT_SUPPORTED[] =
  "Records are not supported here";
constexpr char CALL_ILLFORMED_FUNC[] =
  "calling an ill-formed function pointer without resolved "
  "class/object pointer";
constexpr char WARN_CLS_METH_WRONG_ARGS[] =
  "Arguments to class_meth() must be a literal class name "
  "followed by a literal string "
  "that refers to a static method on that class";

constexpr char CLSMETH_COMPAT_IS_ARR[] = "is_array() called on clsmeth";
constexpr char CLSMETH_COMPAT_IS_VEC[] = "is_vec() called on clsmeth";
constexpr char CLSMETH_COMPAT_IS_VARR[] = "is_varray() called on clsmeth";
constexpr char CLSMETH_COMPAT_IS_ANY_ARR[] = "is_any_array() called on clsmeth";

constexpr char FUNC_TO_STRING_IMPLICIT[] =
  "Implicit Func to string conversion for type-hint";
constexpr char FUNC_TO_STRING[] = "Func to string conversion";
constexpr char CLASS_TO_STRING_IMPLICIT[] =
  "Implicit Class to string conversion for type-hint";
constexpr char CLASS_TO_STRING[] = "Class to string conversion";

} // namespace Strings
} // namespace HPHP

#endif // incl_HPHP_STRINGS_H_
