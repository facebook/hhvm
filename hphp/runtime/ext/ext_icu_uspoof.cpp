/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_icu_uspoof.h"
#include "hphp/runtime/base/exceptions.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(icu_uspoof);
///////////////////////////////////////////////////////////////////////////////

const int64_t q_SpoofChecker$$SINGLE_SCRIPT_CONFUSABLE =
  USPOOF_SINGLE_SCRIPT_CONFUSABLE;
const int64_t q_SpoofChecker$$MIXED_SCRIPT_CONFUSABLE =
  USPOOF_MIXED_SCRIPT_CONFUSABLE;
const int64_t q_SpoofChecker$$WHOLE_SCRIPT_CONFUSABLE =
  USPOOF_WHOLE_SCRIPT_CONFUSABLE;
const int64_t q_SpoofChecker$$ANY_CASE = USPOOF_ANY_CASE;
const int64_t q_SpoofChecker$$SINGLE_SCRIPT = USPOOF_SINGLE_SCRIPT;
const int64_t q_SpoofChecker$$INVISIBLE = USPOOF_INVISIBLE;
const int64_t q_SpoofChecker$$CHAR_LIMIT = USPOOF_CHAR_LIMIT;

///////////////////////////////////////////////////////////////////////////////
c_SpoofChecker::c_SpoofChecker(Class* cb) :
    ExtObjectData(cb) {
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

void c_SpoofChecker::t___construct() {
}

bool c_SpoofChecker::t_issuspicious(const String& text, VRefParam issuesFound) {
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
  const String& s1,
  const String& s2,
  VRefParam issuesFound) {
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

void c_SpoofChecker::t_setallowedlocales(const String& localesList) {
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

///////////////////////////////////////////////////////////////////////////////
}
