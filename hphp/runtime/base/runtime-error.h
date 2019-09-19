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

#ifndef incl_HPHP_RUNTIME_ERROR_H_
#define incl_HPHP_RUNTIME_ERROR_H_

#include <cstdarg>
#include <string>

#include "hphp/util/portability.h"
#include "hphp/runtime/base/annot-type.h"
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

enum class HackStrictOption;

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
    DataType expected_type,
    DataType actual_type);
void raise_param_type_warning(
    const char* func_name,
    int param_num,
    DataType expected_type,
    DataType actual_type);
void raise_hack_strict(HackStrictOption option, const char *ini_setting,
                       const std::string& msg);
void raise_hack_strict(HackStrictOption option, const char *ini_setting,
                       const char *fmt, ...);

/*
 * raise_typehint_error() is the same as raise_recoverable_error(), except
 * the error handler is not allowed to recover.
 * raise_reified_typehint_error flavor also takes a warn flag that demotes the
 * error to a warning for reified generics migrations purposes
 */
void raise_typehint_error(const std::string& msg);
void raise_reified_typehint_error(const std::string& msg, bool warn);

/*
 * raise_return_typehint_error() is the same as raise_recoverable_error(),
 * except when compiled in RepoAuthoritative mode with CheckReturnTypeHints >= 3
 * the error handler is not allowed to recover.
 */
void raise_return_typehint_error(const std::string& msg);

/*
 * Raise the appropriate warning or error (with the given message) for some
 * violation of a property type-hint. If isSoft is true, than a warning is
 * always raised.
 */
void raise_property_typehint_error(const std::string& msg, bool isSoft);

/*
 * Raise the appropriate warning or error (with the given message) for some
 * violation of a record field type-hint. If isSoft is true, than a warning is
 * always raised.
 */
void raise_record_field_typehint_error(const std::string& msg, bool isSoft);

/*
 * Raise  error if a record field is not inititialized after construction.
 */
void raise_record_init_error(const StringData* recName,
                             const StringData* fieldName);

/*
 * Raise error if a record field is not declared.
 */
[[noreturn]] void raise_record_field_error(const StringData* recName,
                                           const StringData* fieldName);

/*
 * Raise the appropriate warning or error if we try to bind a property to a ref,
 * and that property has a type-hint which we're enforcing.
 */
void raise_property_typehint_binding_error(const Class* declCls,
                                           const StringData* propName,
                                           bool isSoft);

/*
 * Raise the appropriate warning or error if we try to unset a property, and
 * that property has a type-hint which we're enforcing.
 */
void raise_property_typehint_unset_error(const Class* declCls,
                                         const StringData* propName,
                                         bool isSoft);

void raise_resolve_undefined(const StringData* name, const Class* c = nullptr);
[[noreturn]] void raise_call_to_undefined(const StringData* name,
                                          const Class* c = nullptr);

void raise_convert_object_to_string(const char* cls_name);
void raise_convert_record_to_type(const char* typeName);

///////////////////////////////////////////////////////////////////////////////
/*
 * Hack arrays compat notices.
 */

void raise_hack_arr_compat_serialize_notice(const ArrayData*);

void raise_hack_arr_compat_array_producing_func_notice(const std::string& name);

void raise_hackarr_compat_type_hint_param_notice(const Func* func,
                                                 const ArrayData* ad,
                                                 const char* name,
                                                 int param);
void raise_hackarr_compat_type_hint_ret_notice(const Func* func,
                                               const ArrayData* ad,
                                               const char* name);
void raise_hackarr_compat_type_hint_outparam_notice(const Func* func,
                                                    const ArrayData* ad,
                                                    const char* name,
                                                    int param);
void raise_hackarr_compat_type_hint_property_notice(const Class* declCls,
                                                    const ArrayData* ad,
                                                    const char* name,
                                                    const StringData* propName,
                                                    bool isStatic);
void raise_hackarr_compat_type_hint_rec_field_notice(
    const StringData* recName,
    const ArrayData* ad,
    const char* name,
    const StringData* fieldName);
void raise_hackarr_compat_is_operator(const char* source, const char* target);

void raise_hackarr_compat_notice(const std::string& msg);

void raise_array_serialization_notice(const char* src, const ArrayData* arr);

#define HC(Opt, opt) void raise_hac_##opt##_notice(const std::string& msg);
HAC_CHECK_OPTS
#undef HC

/*
 * RAII mechanism to temporarily suppress a specific Hack array compat notice
 * within a scope.
 */
#define HC(Opt, ...) \
  struct SuppressHAC##Opt##Notices {  \
    SuppressHAC##Opt##Notices();      \
    ~SuppressHAC##Opt##Notices();     \
    SuppressHAC##Opt##Notices(const SuppressHAC##Opt##Notices&) = delete; \
    SuppressHAC##Opt##Notices(SuppressHAC##Opt##Notices&&) = delete;      \
    SuppressHAC##Opt##Notices&                              \
      operator=(const SuppressHAC##Opt##Notices&) = delete; \
    SuppressHAC##Opt##Notices&                              \
      operator=(SuppressHAC##Opt##Notices&&) = delete;      \
  private:    \
    bool old; \
  };
HAC_CHECK_OPTS
#undef HC

///////////////////////////////////////////////////////////////////////////////

void raise_str_to_class_notice(const StringData* name);

/*
 * class_meth compact notices.
 */
void raise_clsmeth_compat_type_hint(
  const Func* func, const std::string& displayName, folly::Optional<int> param);
void raise_clsmeth_compat_type_hint_outparam_notice(
  const Func* func, const std::string& displayName, int paramNum);
void raise_clsmeth_compat_type_hint_property_notice(
  const Class* declCls, const StringData* propName,
  const std::string& displayName, bool isStatic);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_RUNTIME_ERROR_H_
