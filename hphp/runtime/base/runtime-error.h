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

#include <cstdarg>
#include <string>

#include "hphp/util/portability.h"
#include "hphp/runtime/base/datatype.h"

#ifdef ERROR
#undef ERROR
#endif

#ifdef STRICT
#undef STRICT
#endif

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct Class;
struct Func;
struct ArrayData;
struct StringData;
struct TypeConstraint;
struct TypedValue;

enum class ErrorMode {
  ERROR = 1,
  WARNING = 2,
  PARSE = 4, // not supported
  NOTICE = 8,
  CORE_ERROR = 16, // not supported
  CORE_WARNING = 32, // not supported
  COMPILE_ERROR = 64, // not supported
  COMPILE_WARNING = 128, // not supported
  USER_ERROR = 256,
  USER_WARNING = 512,
  USER_NOTICE = 1024,
  STRICT = 2048,
  RECOVERABLE_ERROR = 4096,
  PHP_DEPRECATED = 8192, // DEPRECATED conflicts with macro definitions
  USER_DEPRECATED = 16384,

  /*
   * PHP's fatal errors cannot be fed into error handler. HipHop can. We
   * still need "ERROR" bit, so old PHP error handler can see this error.
   * The extra 24th bit will help people who want to find out if it's
   * a fatal error only HipHop throws or not.
   */
  FATAL_ERROR = ERROR | (1 << 24), // 16777217

  PHP_ALL = ERROR | WARNING | PARSE | NOTICE | CORE_ERROR | CORE_WARNING |
    COMPILE_ERROR | COMPILE_WARNING | USER_ERROR | USER_WARNING |
    USER_NOTICE | RECOVERABLE_ERROR | PHP_DEPRECATED | USER_DEPRECATED,

  HPHP_ALL = PHP_ALL | FATAL_ERROR,

  /* Errors that can be upgraded to E_USER_ERROR. */
  UPGRADEABLE_ERROR = WARNING | USER_WARNING | NOTICE | USER_NOTICE
};

[[noreturn]] void raise_error(const std::string&);
[[noreturn]] void raise_error(ATTRIBUTE_PRINTF_STRING const char*, ...)
  ATTRIBUTE_PRINTF(1, 2);
[[noreturn]] void raise_error_without_first_frame(const std::string&);
void raise_recoverable_error(const std::string &msg);
void raise_recoverable_error(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
  ATTRIBUTE_PRINTF(1, 2);
void raise_recoverable_error_without_first_frame(const std::string &msg);
void raise_strict_warning(const std::string &msg);
void raise_strict_warning(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
  ATTRIBUTE_PRINTF(1, 2);
void raise_strict_warning_without_first_frame(const std::string &msg);
void raise_warning(const std::string &msg);
void raise_warning(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
  ATTRIBUTE_PRINTF(1, 2);
void raise_warning_without_first_frame(const std::string &msg);
void raise_notice(const std::string &msg);
void raise_notice(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
  ATTRIBUTE_PRINTF(1, 2);
void raise_notice_without_first_frame(const std::string &msg);
void raise_deprecated(const std::string &msg);
void raise_deprecated(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
  ATTRIBUTE_PRINTF(1, 2);
void raise_deprecated_without_first_frame(const std::string &msg);
void raise_warning_unsampled(const std::string &msg);
void raise_warning_unsampled(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
  ATTRIBUTE_PRINTF(1, 2);
void raise_message(ErrorMode mode, const char *fmt, va_list ap);
void raise_message(ErrorMode mode,
  ATTRIBUTE_PRINTF_STRING const char *fmt, ...) ATTRIBUTE_PRINTF(2, 3);
void raise_message(ErrorMode mode, bool skipTop, const std::string& msg);
std::string param_type_error_message(
    const char* func_name,
    int param_num,
    const char* expected_type,
    TypedValue actual_value);
void raise_param_type_warning(
    const char* func_name,
    int param_num,
    const char* expected_type,
    TypedValue actual_value);

/*
 * raise_typehint_error() is the same as raise_recoverable_error(), except
 * the error handler is not allowed to recover.
 * raise_reified_typehint_error flavor also takes a warn flag that demotes the
 * error to a warning for reified generics migrations purposes
 */
[[noreturn]] void raise_typehint_error(const std::string& msg);
void raise_typehint_error_without_first_frame(const std::string& msg);
void raise_reified_typehint_error(const std::string& msg, bool warn);

/*
 * raise_return_typehint_error() is the same as raise_recoverable_error(),
 * except the error handler is not allowed to recover.
 */
void raise_return_typehint_error(const std::string& msg);

/*
 * Raise the appropriate warning or error (with the given message) for some
 * violation of a property type-hint. If isSoft is true, than a warning is
 * always raised.
 */
void raise_property_typehint_error(const std::string& msg,
                                   bool isSoft, bool isUB);

/*
 * Raise the appropriate warning or error if we try to unset a property, and
 * that property has a type-hint which we're enforcing.
 */
void raise_property_typehint_unset_error(const Class* declCls,
                                         const StringData* propName,
                                         bool isSoft, bool isUB);

[[noreturn]] void raise_resolve_func_undefined(const StringData* name, const Class* c = nullptr);
[[noreturn]] void raise_call_to_undefined(const StringData* name,
                                          const Class* c = nullptr);
[[noreturn]] void raise_resolve_class_undefined(const StringData* name);

[[noreturn]] void raise_convert_object_to_string(const char* cls_name);
[[noreturn]] void throw_convert_rfunc_to_type(const char* typeName);
[[noreturn]] void throw_convert_rcls_meth_to_type(const char* typeName);
[[noreturn]] void throw_convert_ecl_to_type(const char* typeName);

///////////////////////////////////////////////////////////////////////////////
/*
 * Hack arrays compat notices.
 */

void raise_hack_arr_compat_serialize_notice(const ArrayData*);

void raise_hackarr_compat_is_operator(const char* source, const char* target);

void raise_hackarr_compat_notice(const std::string& msg);

[[noreturn]] void raise_use_of_specialized_array();

void raise_class_to_string_conversion_notice(const char* source);
void raise_class_to_memokey_conversion_warning();

/*
 * RAII mechanism to temporarily suppress lazyclass-to-string conversion notices
 * within a scope.
 */
struct SuppressClassConversionNotice {
  SuppressClassConversionNotice();
  ~SuppressClassConversionNotice();
  SuppressClassConversionNotice(const SuppressClassConversionNotice&)
      = delete;
  SuppressClassConversionNotice(SuppressClassConversionNotice&&) = delete;
  SuppressClassConversionNotice&
      operator=(const SuppressClassConversionNotice&) = delete;
  SuppressClassConversionNotice&
      operator=(SuppressClassConversionNotice&&) = delete;
private:
  bool old;
};


///////////////////////////////////////////////////////////////////////////////

void raise_str_to_class_notice(const StringData* name);

/*
 * class_meth compact notices.
 */
void raise_clsmeth_compat_type_hint(
  const Func* func, const std::string& displayName, Optional<int> param);
void raise_clsmeth_compat_type_hint_outparam_notice(
  const Func* func, const std::string& displayName, int paramNum);
void raise_clsmeth_compat_type_hint_property_notice(
  const Class* declCls, const StringData* propName,
  const std::string& displayName, bool isStatic);

///////////////////////////////////////////////////////////////////////////////
}
