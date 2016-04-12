/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
  "Constants may only evaluate to scalar values";
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
const char* const NULLSAFE_PROP_WRITE_ERROR =
  "?-> is not allowed in write context";
const char* const NULLSAFE_THIS_BASE_ERROR = "?-> is not allowed with $this";
const char* const FUNCTION_NAME_MUST_BE_STRING =
  "Function name must be a string";
const char* const METHOD_NAME_MUST_BE_STRING =
  "Method name must be a string";
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

} // namespace Strings
} // namespace HPHP

#endif // incl_HPHP_STRINGS_H_
