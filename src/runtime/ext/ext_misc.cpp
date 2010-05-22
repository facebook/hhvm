/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
#include <runtime/base/util/exceptions.h>
#include <runtime/base/zend/zend_pack.h>
#include <runtime/base/hphp_system.h>
#include <runtime/eval/parser/scanner.h>
#include <runtime/eval/parser/parser.h>
#include <runtime/base/runtime_option.h>
#include <util/preprocess.h>

namespace HPHP {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

int f_connection_aborted() {
  return f_connection_status() == k_CONNECTION_ABORTED;
}

int f_connection_status() {
  switch (g_context->getConnectionStatus()) {
  case ExecutionContext::Normal:   return k_CONNECTION_NORMAL;
  case ExecutionContext::Aborted:  return k_CONNECTION_ABORTED;
  case ExecutionContext::TimedOut: return k_CONNECTION_TIMEOUT;
  default:
    ASSERT(false);
    break;
  }
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
      String parent_class = FrameInjection::GetParentClassName("constant");
      if (parent_class.empty()) {
        throw FatalErrorException("Cannot access parent");
      } else {
        className = parent_class;
      }
    }
    // taking care of volatile class
    Array arg(CREATE_VECTOR1(className));
    if (invoke("class_exists", arg)) {
      return get_class_constant(className.c_str(), constantName, false);
    } else {
      return null;
    }
  } else {
    const ClassInfo::ConstantInfo *cinfo = ClassInfo::FindConstant(name.data());
    // system/uniquely defined scalar constant (must be valid)
    if (cinfo) return cinfo->value;
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
      String parent_class = FrameInjection::GetParentClassName("defined");
      if (parent_class.empty()) {
        throw FatalErrorException("Cannot access parent");
      } else {
        className = parent_class;
      }
    }
    if (f_class_exists(className)) { // taking care of volatile class
      const ClassInfo::ClassInfo *info;
      for (const char *parentClass = className.data();
           parentClass && *parentClass;
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
    if (ClassInfo::FindConstant(name.data())) return true;
    // dynamic/redeclared constant
    return ((Globals*)get_global_variables())->defined(name.data());
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
  sleep(seconds);
  return 0;
}

void f_usleep(int micro_seconds) {
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
    snprintf(uniqid, sizeof(uniqid), "%s%08x%05x%08x",
             (const char *)prefix, sec, usec, rand());
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
  Lock lock(Eval::Parser::s_lock);

  const char *input = source.data();
  istringstream iss(input);
  stringstream ss;

  istream *is = RuntimeOption::EnableXHP ? preprocessXHP(iss, ss, input) : &iss;
  Eval::Scanner scanner(new ylmm::basic_buffer(*is, false, true),
                        true, false, true);
  Eval::Token tok;
  ylmm::basic_location loc;
  int tokid;
  Array res;
  while ((tokid = scanner.getNextToken(tok, loc))) {
    if (tokid < 256) {
      res.append(String::FromChar((char)tokid));
    } else {
      Array p = CREATE_VECTOR3(tokid, String(tok.getText()), loc.last_line());
      res.append(p);
    }
  }
  return res;
}

String f_token_name(int64 token) {
  switch (token) {
  case 258: return "T_REQUIRE_ONCE";
  case 259: return "T_REQUIRE";
  case 260: return "T_EVAL";
  case 261: return "T_INCLUDE_ONCE";
  case 262: return "T_INCLUDE";
  case 263: return "T_LOGICAL_OR";
  case 264: return "T_LOGICAL_XOR";
  case 265: return "T_LOGICAL_AND";
  case 266: return "T_PRINT";
  case 267: return "T_SR_EQUAL";
  case 268: return "T_SL_EQUAL";
  case 269: return "T_XOR_EQUAL";
  case 270: return "T_OR_EQUAL";
  case 271: return "T_AND_EQUAL";
  case 272: return "T_MOD_EQUAL";
  case 273: return "T_CONCAT_EQUAL";
  case 274: return "T_DIV_EQUAL";
  case 275: return "T_MUL_EQUAL";
  case 276: return "T_MINUS_EQUAL";
  case 277: return "T_PLUS_EQUAL";
  case 278: return "T_BOOLEAN_OR";
  case 279: return "T_BOOLEAN_AND";
  case 280: return "T_IS_NOT_IDENTICAL";
  case 281: return "T_IS_IDENTICAL";
  case 282: return "T_IS_NOT_EQUAL";
  case 283: return "T_IS_EQUAL";
  case 284: return "T_IS_GREATER_OR_EQUAL";
  case 285: return "T_IS_SMALLER_OR_EQUAL";
  case 286: return "T_SR";
  case 287: return "T_SL";
  case 288: return "T_INSTANCEOF";
  case 289: return "T_UNSET_CAST";
  case 290: return "T_BOOL_CAST";
  case 291: return "T_OBJECT_CAST";
  case 292: return "T_ARRAY_CAST";
  case 293: return "T_STRING_CAST";
  case 294: return "T_DOUBLE_CAST";
  case 295: return "T_INT_CAST";
  case 296: return "T_DEC";
  case 297: return "T_INC";
  case 298: return "T_CLONE";
  case 299: return "T_NEW";
  case 300: return "T_EXIT";
  case 301: return "T_IF";
  case 302: return "T_ELSEIF";
  case 303: return "T_ELSE";
  case 304: return "T_ENDIF";
  case 305: return "T_LNUMBER";
  case 306: return "T_DNUMBER";
  case 307: return "T_STRING";
  case 308: return "T_STRING_VARNAME";
  case 309: return "T_VARIABLE";
  case 310: return "T_NUM_STRING";
  case 311: return "T_INLINE_HTML";
  case 312: return "T_CHARACTER";
  case 313: return "T_BAD_CHARACTER";
  case 314: return "T_ENCAPSED_AND_WHITESPACE";
  case 315: return "T_CONSTANT_ENCAPSED_STRING";
  case 316: return "T_ECHO";
  case 317: return "T_DO";
  case 318: return "T_WHILE";
  case 319: return "T_ENDWHILE";
  case 320: return "T_FOR";
  case 321: return "T_ENDFOR";
  case 322: return "T_FOREACH";
  case 323: return "T_ENDFOREACH";
  case 324: return "T_DECLARE";
  case 325: return "T_ENDDECLARE";
  case 326: return "T_AS";
  case 327: return "T_SWITCH";
  case 328: return "T_ENDSWITCH";
  case 329: return "T_CASE";
  case 330: return "T_DEFAULT";
  case 331: return "T_BREAK";
  case 332: return "T_CONTINUE";
  case 333: return "T_FUNCTION";
  case 334: return "T_CONST";
  case 335: return "T_RETURN";
  case 336: return "T_TRY";
  case 337: return "T_CATCH";
  case 338: return "T_THROW";
  case 339: return "T_USE";
  case 340: return "T_GLOBAL";
  case 341: return "T_PUBLIC";
  case 342: return "T_PROTECTED";
  case 343: return "T_PRIVATE";
  case 344: return "T_FINAL";
  case 345: return "T_ABSTRACT";
  case 346: return "T_STATIC";
  case 347: return "T_VAR";
  case 348: return "T_UNSET";
  case 349: return "T_ISSET";
  case 350: return "T_EMPTY";
  case 351: return "T_HALT_COMPILER";
  case 352: return "T_CLASS";
  case 353: return "T_INTERFACE";
  case 354: return "T_EXTENDS";
  case 355: return "T_IMPLEMENTS";
  case 356: return "T_OBJECT_OPERATOR";
  case 357: return "T_DOUBLE_ARROW";
  case 358: return "T_LIST";
  case 359: return "T_ARRAY";
  case 360: return "T_CLASS_C";
  case 361: return "T_METHOD_C";
  case 362: return "T_FUNC_C";
  case 363: return "T_LINE";
  case 364: return "T_FILE";
  case 365: return "T_COMMENT";
  case 366: return "T_DOC_COMMENT";
  case 367: return "T_OPEN_TAG";
  case 368: return "T_OPEN_TAG_WITH_ECHO";
  case 369: return "T_CLOSE_TAG";
  case 370: return "T_WHITESPACE";
  case 371: return "T_START_HEREDOC";
  case 372: return "T_END_HEREDOC";
  case 373: return "T_DOLLAR_OPEN_CURLY_BRACES";
  case 374: return "T_CURLY_OPEN";
  case 375: return "T_PAAMAYIM_NEKUDOTAYIM";
  default:
    return "UNKNOWN";
  }
}

///////////////////////////////////////////////////////////////////////////////
}
