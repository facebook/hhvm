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

#include "hphp/runtime/ext/ext_misc.h"

#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/zend-pack.h"
#include "hphp/runtime/base/hphp-system.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/ext/ext_class.h"
#include "hphp/runtime/ext/ext_math.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/parser/scanner.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP {

using Transl::CallerFrame;

// Make sure "tokenizer" gets added to the list of extensions
IMPLEMENT_DEFAULT_EXTENSION(tokenizer);

const double k_INF = std::numeric_limits<double>::infinity();
const double k_NAN = std::numeric_limits<double>::quiet_NaN();

///////////////////////////////////////////////////////////////////////////////

int64_t f_connection_aborted() {
  return f_connection_status() == k_CONNECTION_ABORTED;
}

int64_t f_connection_status() {
  return k_CONNECTION_NORMAL;
}

int64_t f_connection_timeout() {
  return f_connection_status() == k_CONNECTION_TIMEOUT;
}

static Class* getClassByName(const char* name, int len) {
  Class* cls = nullptr;
  // translate "self" or "parent"
  if (len == 4 && !memcmp(name, "self", 4)) {
    cls = g_vmContext->getContextClass();
    if (!cls) {
      throw FatalErrorException("Cannot access self:: "
                                "when no class scope is active");
    }
  } else if (len == 6 && !memcmp(name, "parent", 6)) {
    cls = g_vmContext->getParentContextClass();
    if (!cls) {
      throw FatalErrorException("Cannot access parent");
    }
  } else if (len == 6 && !memcmp(name, "static", 6)) {
    CallerFrame cf;
    auto ar = cf();
    if (ar) {
      if (ar->hasThis()) {
        cls = ar->getThis()->getVMClass();
      } else if (ar->hasClass()) {
        cls = ar->getClass();
      }
    }
    if (!cls) {
      throw FatalErrorException("Cannot access static:: "
                                "when no class scope is active");
    }
  } else {
    String className(name, len, CopyString);
    cls = Unit::loadClass(className.get());
  }
  return cls;
}

Variant f_constant(CStrRef name) {
  if (!name.get()) return uninit_null();
  const char *data = name.data();
  int len = name.length();

  // slice off starting backslash
  bool hadInitialBackslash = false;
  if (len > 0 && data[0] == '\\') {
    data += 1;
    len -= 1;
    hadInitialBackslash = true;
  }

  char *colon;
  if ((colon = (char*)memchr(data, ':', len)) && colon[1] == ':') {
    // class constant
    int classNameLen = colon - data;
    char *constantName = colon + 2;
    Class* cls = getClassByName(data, classNameLen);
    if (cls) {
      String cnsName(constantName, data + len - constantName, CopyString);
      TypedValue* tv = cls->clsCnsGet(cnsName.get());
      if (tv) {
        return tvAsCVarRef(tv);
      }
    }
    raise_warning("Couldn't find constant %s", data);
  } else {
    TypedValue* cns;
    if (hadInitialBackslash) {
      String s(data, len, CopyString);
      cns = Unit::loadCns(s.get());
    } else {
      cns = Unit::loadCns(name.get());
    }
    if (cns) return tvAsVariant(cns);
  }

  return uninit_null();
}

bool f_define(CStrRef name, CVarRef value,
              bool case_insensitive /* = false */) {
  if (case_insensitive) {
    raise_warning(Strings::CONSTANTS_CASE_SENSITIVE);
  }
  return Unit::defCns(name.get(), value.asCell());
}

bool f_defined(CStrRef name, bool autoload /* = true */) {
  if (!name.get()) return false;
  const char *data = name.data();
  int len = name.length();

  // slice off starting backslash
  bool hadInitialBackslash = false;
  if (len > 0 && data[0] == '\\') {
    data += 1;
    len -= 1;
    hadInitialBackslash = true;
  }

  char *colon;
  if ((colon = (char*)memchr(data, ':', len)) && colon[1] == ':') {
    // class constant
    int classNameLen = colon - data;
    char *constantName = colon + 2;
    Class* cls = getClassByName(data, classNameLen);
    if (cls) {
      String cnsName(constantName, data + len - constantName, CopyString);
      return cls->clsCnsGet(cnsName.get());
    }
    return false;
  } else {
    auto* cb = autoload ? Unit::loadCns : Unit::lookupCns;
    if (hadInitialBackslash) {
      String s(data, len, CopyString);
      return cb(s.get());
    } else {
      return cb(name.get());
    }
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
  String prefixedCode = concat("<?php ", code_str);
  Unit* unit = g_vmContext->compileEvalString(prefixedCode.get());
  TypedValue retVal;
  g_vmContext->invokeUnit(&retVal, unit);
  return tvAsVariant(&retVal);
}

Variant f_get_browser(CStrRef user_agent /* = null_string */,
                      bool return_array /* = false */) {
  throw NotSupportedException(__func__, "bad idea");
}

void f___halt_compiler() {
  // do nothing
}

Variant f_show_source(CStrRef filename, bool ret /* = false */) {
  throw NotSupportedException(__func__, "PHP specific");
}

int64_t f_ignore_user_abort(bool setting /* = false */) {
  return 0;
}

Variant f_pack(int _argc, CStrRef format, CArrRef _argv /* = null_array */) {
  return ZendPack().pack(format, _argv);
}

bool f_php_check_syntax(CStrRef filename, VRefParam error_message /* = null */) {
  throw NotSupportedException(__func__, "PHP specific");
}

int64_t f_sleep(int seconds) {
  IOStatusHelper io("sleep");
  sleep(seconds);
  return 0;
}

void f_usleep(int micro_seconds) {
  IOStatusHelper io("usleep");
  usleep(micro_seconds);
}

const StaticString
  s_seconds("seconds"),
  s_nanoseconds("nanoseconds");

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
    return CREATE_MAP2(s_seconds, (int64_t)rem.tv_sec,
                       s_nanoseconds, (int64_t)rem.tv_nsec);
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
             prefix.c_str(), sec, usec, math_combined_lcg() * 10);
  } else {
    snprintf(uniqid, sizeof(uniqid), "%s%08x%05x",
             prefix.c_str(), sec, usec);
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
                  RuntimeOption::GetScannerType() | Scanner::ReturnAllTokens);
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

String f_token_name(int64_t token) {

#undef YYTOKENTYPE
#ifdef YYTOKEN_MAP
#undef YYTOKEN_MAP
#undef YYTOKEN
#endif
#define YYTOKEN(num, name) #name
#define YYTOKEN_MAP static const char *names[] =
#include "hphp/parser/hphp.tab.hpp"
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

String f_hphp_to_string(CVarRef v) {
  return v.toString();
}

///////////////////////////////////////////////////////////////////////////////
}
