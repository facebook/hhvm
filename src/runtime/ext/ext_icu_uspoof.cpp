/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include <runtime/ext/ext_icu_uspoof.h>
#include <runtime/base/util/exceptions.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(icu_uspoof);
///////////////////////////////////////////////////////////////////////////////

const int q_SpoofChecker_SINGLE_SCRIPT_CONFUSABLE =
  USPOOF_SINGLE_SCRIPT_CONFUSABLE;
const int q_SpoofChecker_MIXED_SCRIPT_CONFUSABLE =
  USPOOF_MIXED_SCRIPT_CONFUSABLE;
const int q_SpoofChecker_WHOLE_SCRIPT_CONFUSABLE =
  USPOOF_WHOLE_SCRIPT_CONFUSABLE;
const int q_SpoofChecker_ANY_CASE = USPOOF_ANY_CASE;
const int q_SpoofChecker_SINGLE_SCRIPT = USPOOF_SINGLE_SCRIPT;
const int q_SpoofChecker_INVISIBLE = USPOOF_INVISIBLE;
const int q_SpoofChecker_CHAR_LIMIT = USPOOF_CHAR_LIMIT;

///////////////////////////////////////////////////////////////////////////////
c_SpoofChecker::c_SpoofChecker() {
  UErrorCode status = U_ZERO_ERROR;
  m_spoof_checker = uspoof_open(&status);

  // Any of the subsequent calls will be no-ops if 'status' indicates failure.
  //
  // Single-script enforcement is on by default. This fails for languages
  // like Japanese that legally use multiple scripts within a single word,
  // so we turn it off.
  int32_t checks = uspoof_getChecks(
    m_spoof_checker,
    &status);

  uspoof_setChecks(
    m_spoof_checker,
    checks & ~USPOOF_SINGLE_SCRIPT,
    &status);

  if (U_FAILURE(status)) {
    throw Exception("Could not open spoof checker, error %d (%s)",
                    status, u_errorName(status));
  }
}

c_SpoofChecker::~c_SpoofChecker() {
  uspoof_close(m_spoof_checker);
}

void c_SpoofChecker::t___construct() {
}

bool c_SpoofChecker::t_issuspicious(CStrRef text, Variant issuesFound) {
  INSTANCE_METHOD_INJECTION_BUILTIN(SpoofChecker, SpoofChecker::issuspicious);
  UErrorCode status = U_ZERO_ERROR;
  int32_t ret = uspoof_checkUTF8(
    m_spoof_checker,
    text.data(),
    text.length(),
    NULL,
    &status);
  if (U_FAILURE(status)) {
    throw Exception(
      "Could not check [%s] for security issues, error %d (%s)",
      text.c_str(), status, u_errorName(status));
  }
  issuesFound = ret;
  return ret != 0;
}

bool c_SpoofChecker::t_areconfusable(
  CStrRef s1,
  CStrRef s2,
  Variant issuesFound) {
  INSTANCE_METHOD_INJECTION_BUILTIN(SpoofChecker, SpoofChecker::areconfusable);
  UErrorCode status = U_ZERO_ERROR;
  int32_t ret = uspoof_areConfusableUTF8(
    m_spoof_checker,
    s1.data(),
    s1.length(),
    s2.data(),
    s2.length(),
    &status);
  if (U_FAILURE(status)) {
    throw Exception(
      "Could not check [%s] and [%s] for confusability, error %d (%s)",
      s1.c_str(), s2.c_str(), status, u_errorName(status));
  }
  issuesFound = ret;
  return ret != 0;
}

void c_SpoofChecker::t_setallowedlocales(CStrRef localesList) {
  INSTANCE_METHOD_INJECTION_BUILTIN(SpoofChecker, SpoofChecker::setallowedlocales);
  UErrorCode status = U_ZERO_ERROR;
  uspoof_setAllowedLocales(
    m_spoof_checker,
    localesList.c_str(),
    &status);
  if (U_FAILURE(status)) {
    throw Exception(
      "Could not set allowed locales to [%s], error %d (%s)",
      localesList.c_str(), status, u_errorName(status));
  }
}

void c_SpoofChecker::t_setchecks(int checks) {
  INSTANCE_METHOD_INJECTION_BUILTIN(SpoofChecker, SpoofChecker::setchecks);
  UErrorCode status = U_ZERO_ERROR;
  uspoof_setChecks(
    m_spoof_checker,
    checks,
    &status);
  if (U_FAILURE(status)) {
    throw Exception(
      "Could not set spoof checks to %d, error %d (%s)",
      checks, status, u_errorName(status));
  }
}

Variant c_SpoofChecker::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(SpoofChecker, SpoofChecker::__destruct);
  return null;
}

///////////////////////////////////////////////////////////////////////////////
}
