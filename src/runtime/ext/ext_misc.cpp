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

#include <runtime/ext/ext_misc.h>
#include <runtime/ext/ext_class.h>
#include <runtime/ext/ext_math.h>
#include <runtime/base/util/exceptions.h>
#include <runtime/base/zend/zend_pack.h>
#include <runtime/base/hphp_system.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/server_stats.h>
#include <util/parser/scanner.h>

namespace HPHP {
using namespace std;

// Make sure "tokenizer" gets added to the list of extensions
IMPLEMENT_DEFAULT_EXTENSION(tokenizer);

const double k_INF = numeric_limits<double>::infinity();
const double k_NAN = numeric_limits<double>::quiet_NaN();

///////////////////////////////////////////////////////////////////////////////

int f_connection_aborted() {
  return f_connection_status() == k_CONNECTION_ABORTED;
}

int f_connection_status() {
  return k_CONNECTION_NORMAL;
}

int f_connection_timeout() {
  return f_connection_status() == k_CONNECTION_TIMEOUT;
}

Variant f_constant(CStrRef name) {
  const char *data = name.data();
  int len = name.length();
  char *colon;
  if ((colon = (char*)memchr(data, ':', len)) && colon[1] == ':') {
    // class constant
    int classNameLen = colon - data;
    char *constantName = colon + 2;
    String className(data, classNameLen, CopyString);

    // translate "self" or "parent"
    if (className == "self") {
      String this_class = FrameInjection::GetClassName(true);
      if (this_class.empty()) {
        throw FatalErrorException("Cannot access self:: "
          "when no class scope is active");
      } else {
        className = this_class;
      }
    } else if (className == "parent") {
      CStrRef parent_class = FrameInjection::GetParentClassName(true);
      if (parent_class.empty()) {
        throw FatalErrorException("Cannot access parent");
      } else {
        className = parent_class;
      }
    }
    // taking care of volatile class
    if (class_exists(className)) {
      return get_class_constant(className.c_str(), constantName, false);
    } else {
      return null;
    }
  } else {
    const ClassInfo::ConstantInfo *cinfo = ClassInfo::FindConstant(name);
    // system/uniquely defined scalar constant (must be valid)
    if (cinfo) return cinfo->getValue();
    // dynamic/redeclared constant
    return ((Globals*)get_global_variables())->getConstant(name.data());
  }
}

bool f_define(CStrRef name, CVarRef value,
              bool case_insensitive /* = false */) {
  ASSERT(false); // define() should be turned into constant definition by HPHP
  return false;
}

bool f_defined(CStrRef name) {
  const char *data = name.data();
  int len = name.length();
  char *colon;
  if ((colon = (char*)memchr(data, ':', len)) && colon[1] == ':') {
    // class constant
    int classNameLen = colon - data;
    char *constantName = colon + 2;
    String className(data, classNameLen, CopyString);

    // translate "self" or "parent"
    if (className == "self") {
      String this_class = FrameInjection::GetClassName(true);
      if (this_class.empty()) {
        throw FatalErrorException("Cannot access self:: "
          "when no class scope is active");
      } else {
        className = this_class;
      }
    } else if (className == "parent") {
      CStrRef parent_class = FrameInjection::GetParentClassName(true);
      if (parent_class.empty()) {
        throw FatalErrorException("Cannot access parent");
      } else {
        className = parent_class;
      }
    }
    if (class_exists(className)) { // taking care of volatile class
      const ClassInfo *info;
      for (String parentClass = className;
           !parentClass.empty();
           parentClass = info->getParentClass()) {
        info = ClassInfo::FindClass(parentClass);
        if (!info) {
          ASSERT(false);
        }
        if (info->hasConstant(constantName)) return true;
      }
      return false;
    } else {
      return false;
    }
  } else {
    // system/uniquely defined scalar constant
    if (ClassInfo::FindConstant(name)) return true;
    // dynamic/redeclared constant
    return ((Globals*)get_global_variables())->defined(name);
  }
}

Variant f_die(CVarRef status /* = null_variant */) {
  return f_exit(status);
}

Variant f_exit(CVarRef status /* = null_variant */) {
  if (status.isString()) {
    echo(status.toString());
    throw ExitException(0);
  }
  throw ExitException(status.toInt32());
}

Variant f_eval(CStrRef code_str) {
  throw NotSupportedException(__func__, "rebuild with eval mode");
}

Variant f_get_browser(CStrRef user_agent /* = null_string */,
                      bool return_array /* = false */) {
  throw NotSupportedException(__func__, "bad idea");
}

void f___halt_compiler() {
  // do nothing
}

Variant f_highlight_file(CStrRef filename, bool ret /* = false */) {
  throw NotSupportedException(__func__, "PHP specific");
}

Variant f_show_source(CStrRef filename, bool ret /* = false */) {
  throw NotSupportedException(__func__, "PHP specific");
}

Variant f_highlight_string(CStrRef str, bool ret /* = false */) {
  throw NotSupportedException(__func__, "PHP specific");
}

int f_ignore_user_abort(bool setting /* = false */) {
  return 0;
}

Variant f_pack(int _argc, CStrRef format, CArrRef _argv /* = null_array */) {
  return ZendPack().pack(format, _argv);
}

bool f_php_check_syntax(CStrRef filename, Variant error_message /* = null */) {
  throw NotSupportedException(__func__, "PHP specific");
}

String f_php_strip_whitespace(CStrRef filename) {
  throw NotSupportedException(__func__, "PHP specific");
}

int f_sleep(int seconds) {
  IOStatusHelper io("sleep");
  sleep(seconds);
  return 0;
}

void f_usleep(int micro_seconds) {
  IOStatusHelper io("usleep");
  usleep(micro_seconds);
}

Variant f_time_nanosleep(int seconds, int nanoseconds) {
  if (seconds < 0) {
    throw_invalid_argument("seconds: cannot be negative");
    return false;
  }
  if (nanoseconds < 0 || nanoseconds > 999999999) {
    throw_invalid_argument("nanoseconds: has to be 0 to 999999999");
    return false;
  }

  struct timespec req, rem;
  req.tv_sec = (time_t)seconds;
  req.tv_nsec = nanoseconds;

  IOStatusHelper io("nanosleep");
  if (!nanosleep(&req, &rem)) {
    return true;
  }
  if (errno == EINTR) {
    return CREATE_MAP2("seconds", (int64)rem.tv_sec,
                       "nanoseconds", (int64)rem.tv_nsec);
  }
  return false;
}

bool f_time_sleep_until(double timestamp) {
  struct timeval tm;
  if (gettimeofday((struct timeval *)&tm, NULL) != 0) {
    return false;
  }

  double c_ts = (double)(timestamp - tm.tv_sec - tm.tv_usec / 1000000.0);
  if (c_ts < 0) {
    throw_invalid_argument
      ("timestamp: Sleep until to time is less than current time");
    return false;
  }

  struct timespec req, rem;
  req.tv_sec = (time_t)c_ts;
  req.tv_nsec = (long)((c_ts - req.tv_sec) * 1000000000.0);

  IOStatusHelper io("nanosleep");
  while (nanosleep(&req, &rem)) {
    if (errno != EINTR) return false;
    req.tv_sec = rem.tv_sec;
    req.tv_nsec = rem.tv_nsec;
  }

  return true;
}

String f_uniqid(CStrRef prefix /* = null_string */,
                bool more_entropy /* = false */) {
  if (!more_entropy) {
    usleep(1);
  }

  struct timeval tv;
  gettimeofday((struct timeval *)&tv, NULL);
  int sec = (int)tv.tv_sec;
  int usec = (int)(tv.tv_usec % 0x100000);

  char uniqid[256];
  if (more_entropy) {
    snprintf(uniqid, sizeof(uniqid), "%s%08x%05x%.8F",
             (const char *)prefix, sec, usec, math_combined_lcg() * 10);
  } else {
    snprintf(uniqid, sizeof(uniqid), "%s%08x%05x",
             (const char *)prefix, sec, usec);
  }
  return String(uniqid, CopyString);
}

Variant f_unpack(CStrRef format, CStrRef data) {
  return ZendPack().unpack(format, data);
}

Array f_sys_getloadavg() {
  double load[3];
  getloadavg(load, 3);
  return CREATE_VECTOR3(load[0], load[1], load[2]);
}


Array f_token_get_all(CStrRef source) {
  Scanner scanner(source.data(), source.size(),
                  Scanner::AllowShortTags | Scanner::ReturnAllTokens);
  ScannerToken tok;
  Location loc;
  int tokid;
  Array res;
  while ((tokid = scanner.getNextToken(tok, loc))) {
    if (tokid < 256) {
      res.append(String::FromChar((char)tokid));
    } else {
      Array p = CREATE_VECTOR3(tokid, String(tok.text()), loc.line0);
      res.append(p);
    }
  }
  return res;
}

String f_token_name(int64 token) {

#undef YYTOKENTYPE
#ifdef YYTOKEN_MAP
#undef YYTOKEN_MAP
#undef YYTOKEN
#endif
#define YYTOKEN(num, name) #name
#define YYTOKEN_MAP static const char *names[] =
#include <util/parser/hphp.tab.hpp>
#undef YYTOKEN_MAP
#undef YYTOKEN

  if (token >= YYTOKEN_MIN && token <= YYTOKEN_MAX) {
    const char *name = names[token - YYTOKEN_MIN];
    if (strncmp(name, "T_HPHP_", sizeof("T_HPHP_") - 1)) {
      return name;
    }
  }
  return "UNKNOWN";
}

///////////////////////////////////////////////////////////////////////////////
}
