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

namespace HPHP {
namespace Strings {

constexpr char FATAL_NULL_THIS[] = "$this is null";
constexpr char WARN_NULL_THIS[] = "Undefined variable: this";
constexpr char FUNCTION_ALREADY_DEFINED[] = "Function already defined: %s";
constexpr char CONSTANT_ALREADY_DEFINED[] = "Constant %s already defined";
constexpr char DIVISION_BY_ZERO[] =
  "Division by zero";
constexpr char UNKNOWN_CLASS[] = "Class undefined: %s";
constexpr char FAILED_RESOLVE_CLASS[] = "Failure to resolve undefined class %s";
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
constexpr char FUNCTION_NAME_MUST_BE_STRING[] =
  "Function name must be a string";
constexpr char METHOD_NAME_MUST_BE_STRING[] =
  "Method name must be a string";
constexpr char CANT_UNSET_STRING[] =
  "Cannot unset string offsets";
constexpr char OP_NOT_SUPPORTED_STRING[] =
  "Operator not supported for strings";
constexpr char OP_NOT_SUPPORTED_FUNC[] =
  "Operator not supported for funcs";
constexpr char OP_NOT_SUPPORTED_CLASS[] =
  "Operator not supported for classes";
constexpr char TRAITS_UNKNOWN_TRAIT[] =
  "Unknown trait '%s'";
constexpr char TRAITS_UNKNOWN_TRAIT_METHOD[] =
  "Unknown trait method '%s'";
constexpr char METHOD_IN_MULTIPLE_TRAITS[] =
  "Method '%s' declared in multiple traits (%s)";
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
constexpr char HACKARR_COMPAT_ARR_HACK_ARR_CMP[] =
  "Comparing PHP array with Hack array";
constexpr char HACKARR_COMPAT_VARR_IS_VEC[] = "is_vec() called on varray";
constexpr char HACKARR_COMPAT_DARR_IS_DICT[] = "is_dict() called on darray";
constexpr char DATATYPE_SPECIALIZED_DVARR[] =
  "Datatype-specialized array currently unsupported";
constexpr char FUNCTION_CALLED_DYNAMICALLY_WITHOUT_ATTRIBUTE[] =
  "'%s' called dynamically but is not __DynamicallyCallable";
constexpr char FUNCTION_CALLED_DYNAMICALLY_WITH_ATTRIBUTE[] =
  "'%s' called dynamically without using a function pointer";
constexpr char CLASS_CONSTRUCTED_DYNAMICALLY[] = "'%s' constructed dynamically";
constexpr char NEW_STATIC_ON_REIFIED_CLASS[] =
  "Cannot call new static since class %s has reified generics";
constexpr char CALL_ILLFORMED_FUNC[] =
  "calling an ill-formed function pointer without resolved "
  "class/object pointer";
constexpr char RFUNC_NOT_SUPPORTED[] =
  "Reified functions not supported";
constexpr char RCLS_METH_NOT_SUPPORTED[] =
  "Reified class method pointers no supported";
constexpr char CLS_METH_NOT_SUPPORTED[] =
  "Class method pointers no supported";

constexpr char CLSMETH_COMPAT_IS_ARR[] = "is_array() called on clsmeth";
constexpr char CLSMETH_COMPAT_IS_VEC[] = "is_vec() called on clsmeth";
constexpr char CLSMETH_COMPAT_IS_ANY_ARR[] = "is_any_array() called on clsmeth";
constexpr char CLSMETH_COMPAT_IS_SHAPE[] = "is_shape() called on clsmeth";
constexpr char CLSMETH_COMPAT_IS_TUPLE[] = "is_tuple() called on clsmeth";
constexpr char CLSMETH_COMPAT_VEC_CMP[] = "Comparing clsmeth with vec";
constexpr char CLSMETH_COMPAT_NON_CLSMETH_REL_CMP[] =
  "Comparing clsmeth with non-clsmeth relationally";

constexpr char FUNC_TO_STRING_IMPLICIT[] =
  "Implicit Func to string conversion for type-hint";
constexpr char FUNC_TO_STRING[] = "Func to string conversion";
constexpr char CLASS_TO_STRING_IMPLICIT[] =
  "Implicit Class to string conversion for %s";
constexpr char CLASS_TO_MEMOKEY[] = "Class to memo key conversion";
constexpr char CLASS_TO_CLASSNAME[] = "Class passed to classname type-hint";
constexpr char ARRAY_MARK_LEGACY_VEC[] = "array_mark_legacy() called on vec";
constexpr char ARRAY_MARK_LEGACY_DICT[] = "array_mark_legacy() called on dict";
constexpr char NONEXHAUSTIVE_SWITCH[] =
  "The switch statement failed to match any of the cases";
constexpr char INVALID_ARGUMENT_FOREACH[] =
  "Invalid argument supplied for foreach()";
constexpr char INVALID_REIFIED_COEFFECT_CLASSNAME[] =
  "Reified generic used for coeffect rule does not refer to a class";
constexpr char READONLY_COLLECTIONS_CANNOT_BE_MODIFIED[] =
  "Readonly collections cannot be modified.";
constexpr char MUST_BE_READONLY[] =
  "Cannot store readonly value in a non-readonly property %s of class %s.";
constexpr char MUST_BE_MUTABLE[] = "Property %s of class %s must be mutable.";
constexpr char MUST_BE_VALUE_TYPE[] = "Property %s of class %s is readonly, and "
  "therefore must be a value type to be modified.";
constexpr char MUST_BE_ENCLOSED_IN_READONLY[] =
  "Property %s of class %s is readonly, but isn't enclosed in a readonly expression.";
constexpr char MISSING_DYNAMICALLY_REFERENCED[] =
  "Missing __DynamicallyReferenced attribute on class %s for classname_to_class";
constexpr char CLASSNAME_TO_CLASS_NOEXIST_EXCEPTION[] =
  "Failed to load class from %s %s for classname_to_class.";

} // namespace Strings
} // namespace HPHP
