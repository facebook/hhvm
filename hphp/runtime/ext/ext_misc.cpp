/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include <limits>

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
#include "hphp/util/logger.h"

namespace HPHP {

IMPLEMENT_THREAD_LOCAL(std::string, s_misc_highlight_default_string);
IMPLEMENT_THREAD_LOCAL(std::string, s_misc_highlight_default_comment);
IMPLEMENT_THREAD_LOCAL(std::string, s_misc_highlight_default_keyword);
IMPLEMENT_THREAD_LOCAL(std::string, s_misc_highlight_default_default);
IMPLEMENT_THREAD_LOCAL(std::string, s_misc_highlight_default_html);
IMPLEMENT_THREAD_LOCAL(std::string, s_misc_display_errors);

const std::string s_1("1"), s_2("2"), s_stdout("stdout"), s_stderr("stderr");

static class MiscExtension : public Extension {
public:
  MiscExtension() : Extension("misc", k_PHP_VERSION.c_str()) { }
  void threadInit() {
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
      "highlight.string", "#DD0000",
      s_misc_highlight_default_string.get()
    );
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
      "highlight.comment", "#FF8000",
      s_misc_highlight_default_comment.get()
    );
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
      "highlight.keyword", "#007700",
      s_misc_highlight_default_keyword.get()
    );
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
      "highlight.default", "#0000BB",
      s_misc_highlight_default_default.get()
    );
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
      "highlight.html", "#000000",
      s_misc_highlight_default_html.get()
    );
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
      "display_errors", "1",
      IniSetting::SetAndGet<std::string>(
        [](const std::string& value) {
          if (value == s_1 || value == s_stdout) {
            Logger::SetStandardOut(stdout);
            return true;
          }
          if (value == s_2 || value == s_stderr) {
            Logger::SetStandardOut(stderr);
            return true;
          }
          return false;
        },
        nullptr
      ),
      s_misc_display_errors.get()
    );
  }

} s_misc_extension;

using JIT::CallerFrame;

// Make sure "tokenizer" gets added to the list of extensions
IMPLEMENT_DEFAULT_EXTENSION_VERSION(tokenizer, 0.1);

const double k_INF = std::numeric_limits<double>::infinity();
const double k_NAN = std::numeric_limits<double>::quiet_NaN();
const bool k_PHP_DEBUG =
#if DEBUG
        true;
#else
        false;
#endif

const int64_t k_PHP_MAXPATHLEN = MAXPATHLEN;

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
    cls = g_context->getContextClass();
    if (!cls) {
      throw FatalErrorException("Cannot access self:: "
                                "when no class scope is active");
    }
  } else if (len == 6 && !memcmp(name, "parent", 6)) {
    cls = g_context->getParentContextClass();
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

Variant f_constant(const String& name) {
  if (!name.get()) return uninit_null();
  const char *data = name.data();
  int len = name.length();

  char *colon;
  if ((colon = (char*)memchr(data, ':', len)) && colon[1] == ':') {
    // class constant
    int classNameLen = colon - data;
    char *constantName = colon + 2;
    Class* cls = getClassByName(data, classNameLen);
    if (cls) {
      String cnsName(constantName, data + len - constantName, CopyString);
      Cell cns = cls->clsCnsGet(cnsName.get());
      if (cns.m_type != KindOfUninit) {
        return cellAsCVarRef(cns);
      }
    }
    raise_warning("Couldn't find constant %s", data);
  } else {
    auto cns = Unit::loadCns(name.get());
    if (cns) return tvAsVariant(cns);
  }

  return uninit_null();
}

bool f_define(const String& name, const Variant& value,
              bool case_insensitive /* = false */) {
  if (case_insensitive) {
    raise_warning(Strings::CONSTANTS_CASE_SENSITIVE);
  }
  return Unit::defCns(name.get(), value.asCell());
}

bool f_defined(const String& name, bool autoload /* = true */) {
  if (!name.get()) return false;
  const char *data = name.data();
  int len = name.length();

  char *colon;
  if ((colon = (char*)memchr(data, ':', len)) && colon[1] == ':') {
    // class constant
    int classNameLen = colon - data;
    char *constantName = colon + 2;
    Class* cls = getClassByName(data, classNameLen);
    if (cls) {
      String cnsName(constantName, data + len - constantName, CopyString);
      return cls->clsCnsGet(cnsName.get()).m_type != KindOfUninit;
    }
    return false;
  } else {
    auto* cb = autoload ? Unit::loadCns : Unit::lookupCns;
    return cb(name.get());
  }
}

Variant f_die(const Variant& status /* = null_variant */) {
  return f_exit(status);
}

Variant f_exit(const Variant& status /* = null_variant */) {
  if (status.isString()) {
    echo(status.toString());
    throw ExitException(0);
  }
  throw ExitException(status.toInt32());
}

void f___halt_compiler() {
  // do nothing
}

int64_t f_ignore_user_abort(bool setting /* = false */) {
  return 0;
}

Variant f_pack(int _argc, const String& format, const Array& _argv /* = null_array */) {
  return ZendPack().pack(format, _argv);
}

int64_t f_sleep(int seconds) {
  IOStatusHelper io("sleep");
  Transport *transport = g_context->getTransport();
  if (transport) {
    transport->incSleepTime(seconds);
  }
  sleep(seconds);
  return 0;
}

void f_usleep(int micro_seconds) {
  IOStatusHelper io("usleep");
  Transport *transport = g_context->getTransport();
  if (transport) {
    transport->incuSleepTime(micro_seconds);
  }
  usleep(micro_seconds);
}

static void recordNanosleepTime(
  const struct timespec &req,
  const struct timespec *rem
) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    int64_t req_s = req.tv_sec;
    int32_t req_n = req.tv_nsec;
    int64_t rem_s = 0;
    int32_t rem_n = 0;

    if (rem) {
      rem_s = rem->tv_sec;
      rem_n = rem->tv_nsec;
    }

    int32_t nanos = req_n - rem_n;
    int64_t seconds = req_s - rem_s;
    if (nanos < 0) {
      nanos += 1000000000;
      seconds--;
    }

    transport->incnSleepTime(seconds, nanos);
  }
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
    recordNanosleepTime(req, nullptr);
    return true;
  }

  recordNanosleepTime(req, &rem);
  if (errno == EINTR) {
    return make_map_array(s_seconds, (int64_t)rem.tv_sec,
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
    recordNanosleepTime(req, &rem);
    if (errno != EINTR) return false;
    req.tv_sec = rem.tv_sec;
    req.tv_nsec = rem.tv_nsec;
  }
  recordNanosleepTime(req, nullptr);
  return true;
}

String f_uniqid(const String& prefix /* = null_string */,
                bool more_entropy /* = false */) {
  if (!more_entropy) {
    Transport *transport = g_context->getTransport();
    if (transport) {
      transport->incuSleepTime(1);
    }
    usleep(1);
  }

  struct timeval tv;
  gettimeofday((struct timeval *)&tv, NULL);
  int sec = (int)tv.tv_sec;
  int usec = (int)(tv.tv_usec % 0x100000);

  String uniqid(prefix.size() + 64, ReserveString);
  auto ptr = uniqid.bufferSlice().ptr;
  auto capacity = uniqid.get()->capacity();
  int64_t len;
  if (more_entropy) {
    len = snprintf(ptr, capacity, "%s%08x%05x%.8F",
                   prefix.c_str(), sec, usec, math_combined_lcg() * 10);
  } else {
    len = snprintf(ptr, capacity, "%s%08x%05x",
                   prefix.c_str(), sec, usec);
  }
  uniqid.setSize(len);
  return uniqid;
}

Variant f_unpack(const String& format, const String& data) {
  return ZendPack().unpack(format, data);
}

Array f_sys_getloadavg() {
  double load[3];
  getloadavg(load, 3);
  return make_packed_array(load[0], load[1], load[2]);
}

// We want token IDs to remain stable regardless of how we change the
// internals of the parser. Thus, we maintain a mapping from internal
// token IDs to stable "user token IDs" and only expose the user token
// IDs to the PHP application.

const int UserTokenId_T_REQUIRE_ONCE = 258;
const int UserTokenId_T_REQUIRE = 259;
const int UserTokenId_T_EVAL = 260;
const int UserTokenId_T_INCLUDE_ONCE = 261;
const int UserTokenId_T_INCLUDE = 262;
const int UserTokenId_T_LOGICAL_OR = 263;
const int UserTokenId_T_LOGICAL_XOR = 264;
const int UserTokenId_T_LOGICAL_AND = 265;
const int UserTokenId_T_PRINT = 266;
const int UserTokenId_T_SR_EQUAL = 267;
const int UserTokenId_T_SL_EQUAL = 268;
const int UserTokenId_T_XOR_EQUAL = 269;
const int UserTokenId_T_OR_EQUAL = 270;
const int UserTokenId_T_AND_EQUAL = 271;
const int UserTokenId_T_MOD_EQUAL = 272;
const int UserTokenId_T_CONCAT_EQUAL = 273;
const int UserTokenId_T_DIV_EQUAL = 274;
const int UserTokenId_T_MUL_EQUAL = 275;
const int UserTokenId_T_MINUS_EQUAL = 276;
const int UserTokenId_T_PLUS_EQUAL = 277;
const int UserTokenId_T_BOOLEAN_OR = 278;
const int UserTokenId_T_BOOLEAN_AND = 279;
const int UserTokenId_T_IS_NOT_IDENTICAL = 280;
const int UserTokenId_T_IS_IDENTICAL = 281;
const int UserTokenId_T_IS_NOT_EQUAL = 282;
const int UserTokenId_T_IS_EQUAL = 283;
const int UserTokenId_T_IS_GREATER_OR_EQUAL = 284;
const int UserTokenId_T_IS_SMALLER_OR_EQUAL = 285;
const int UserTokenId_T_SR = 286;
const int UserTokenId_T_SL = 287;
const int UserTokenId_T_INSTANCEOF = 288;
const int UserTokenId_T_UNSET_CAST = 289;
const int UserTokenId_T_BOOL_CAST = 290;
const int UserTokenId_T_OBJECT_CAST = 291;
const int UserTokenId_T_ARRAY_CAST = 292;
const int UserTokenId_T_STRING_CAST = 293;
const int UserTokenId_T_DOUBLE_CAST = 294;
const int UserTokenId_T_INT_CAST = 295;
const int UserTokenId_T_DEC = 296;
const int UserTokenId_T_INC = 297;
const int UserTokenId_T_CLONE = 298;
const int UserTokenId_T_NEW = 299;
const int UserTokenId_T_EXIT = 300;
const int UserTokenId_T_IF = 301;
const int UserTokenId_T_ELSEIF = 302;
const int UserTokenId_T_ELSE = 303;
const int UserTokenId_T_ENDIF = 304;
const int UserTokenId_T_LNUMBER = 305;
const int UserTokenId_T_DNUMBER = 306;
const int UserTokenId_T_STRING = 307;
const int UserTokenId_T_STRING_VARNAME = 308;
const int UserTokenId_T_VARIABLE = 309;
const int UserTokenId_T_NUM_STRING = 310;
const int UserTokenId_T_INLINE_HTML = 311;
const int UserTokenId_T_CHARACTER = 312;
const int UserTokenId_T_BAD_CHARACTER = 313;
const int UserTokenId_T_ENCAPSED_AND_WHITESPACE = 314;
const int UserTokenId_T_CONSTANT_ENCAPSED_STRING = 315;
const int UserTokenId_T_ECHO = 316;
const int UserTokenId_T_DO = 317;
const int UserTokenId_T_WHILE = 318;
const int UserTokenId_T_ENDWHILE = 319;
const int UserTokenId_T_FOR = 320;
const int UserTokenId_T_ENDFOR = 321;
const int UserTokenId_T_FOREACH = 322;
const int UserTokenId_T_ENDFOREACH = 323;
const int UserTokenId_T_DECLARE = 324;
const int UserTokenId_T_ENDDECLARE = 325;
const int UserTokenId_T_AS = 326;
const int UserTokenId_T_SWITCH = 327;
const int UserTokenId_T_ENDSWITCH = 328;
const int UserTokenId_T_CASE = 329;
const int UserTokenId_T_DEFAULT = 330;
const int UserTokenId_T_BREAK = 331;
const int UserTokenId_T_GOTO = 332;
const int UserTokenId_T_CONTINUE = 333;
const int UserTokenId_T_FUNCTION = 334;
const int UserTokenId_T_CONST = 335;
const int UserTokenId_T_RETURN = 336;
const int UserTokenId_T_TRY = 337;
const int UserTokenId_T_CATCH = 338;
const int UserTokenId_T_THROW = 339;
const int UserTokenId_T_USE = 340;
const int UserTokenId_T_GLOBAL = 341;
const int UserTokenId_T_PUBLIC = 342;
const int UserTokenId_T_PROTECTED = 343;
const int UserTokenId_T_PRIVATE = 344;
const int UserTokenId_T_FINAL = 345;
const int UserTokenId_T_ABSTRACT = 346;
const int UserTokenId_T_STATIC = 347;
const int UserTokenId_T_VAR = 348;
const int UserTokenId_T_UNSET = 349;
const int UserTokenId_T_ISSET = 350;
const int UserTokenId_T_EMPTY = 351;
const int UserTokenId_T_HALT_COMPILER = 352;
const int UserTokenId_T_CLASS = 353;
const int UserTokenId_T_INTERFACE = 354;
const int UserTokenId_T_EXTENDS = 355;
const int UserTokenId_T_IMPLEMENTS = 356;
const int UserTokenId_T_OBJECT_OPERATOR = 357;
const int UserTokenId_T_DOUBLE_ARROW = 358;
const int UserTokenId_T_LIST = 359;
const int UserTokenId_T_ARRAY = 360;
const int UserTokenId_T_CLASS_C = 361;
const int UserTokenId_T_METHOD_C = 362;
const int UserTokenId_T_FUNC_C = 363;
const int UserTokenId_T_LINE = 364;
const int UserTokenId_T_FILE = 365;
const int UserTokenId_T_COMMENT = 366;
const int UserTokenId_T_DOC_COMMENT = 367;
const int UserTokenId_T_OPEN_TAG = 368;
const int UserTokenId_T_OPEN_TAG_WITH_ECHO = 369;
const int UserTokenId_T_CLOSE_TAG = 370;
const int UserTokenId_T_WHITESPACE = 371;
const int UserTokenId_T_START_HEREDOC = 372;
const int UserTokenId_T_END_HEREDOC = 373;
const int UserTokenId_T_DOLLAR_OPEN_CURLY_BRACES = 374;
const int UserTokenId_T_CURLY_OPEN = 375;
const int UserTokenId_T_PAAMAYIM_NEKUDOTAYIM = 376;
const int UserTokenId_T_NAMESPACE = 377;
const int UserTokenId_T_NS_C = 378;
const int UserTokenId_T_DIR = 379;
const int UserTokenId_T_NS_SEPARATOR = 380;
const int UserTokenId_T_YIELD = 381;
const int UserTokenId_T_XHP_LABEL = 382;
const int UserTokenId_T_XHP_TEXT = 383;
const int UserTokenId_T_XHP_ATTRIBUTE = 384;
const int UserTokenId_T_XHP_CATEGORY = 385;
const int UserTokenId_T_XHP_CATEGORY_LABEL = 386;
const int UserTokenId_T_XHP_CHILDREN = 387;
const int UserTokenId_T_XHP_ENUM = 388;
const int UserTokenId_T_XHP_REQUIRED = 389;
const int UserTokenId_T_TRAIT = 390;
const int UserTokenId_T_INSTEADOF = 391;
const int UserTokenId_T_TRAIT_C = 392;
const int UserTokenId_T_VARARG = 393;
const int UserTokenId_T_HH_ERROR = 394;
const int UserTokenId_T_FINALLY = 395;
const int UserTokenId_T_XHP_TAG_LT = 396;
const int UserTokenId_T_XHP_TAG_GT = 397;
const int UserTokenId_T_TYPELIST_LT = 398;
const int UserTokenId_T_TYPELIST_GT = 399;
const int UserTokenId_T_UNRESOLVED_LT = 400;
const int UserTokenId_T_COLLECTION = 401;
const int UserTokenId_T_SHAPE = 402;
const int UserTokenId_T_TYPE = 403;
const int UserTokenId_T_UNRESOLVED_TYPE = 404;
const int UserTokenId_T_NEWTYPE = 405;
const int UserTokenId_T_UNRESOLVED_NEWTYPE = 406;
const int UserTokenId_T_COMPILER_HALT_OFFSET = 407;
const int UserTokenId_T_AWAIT = 408;
const int UserTokenId_T_ASYNC = 409;
const int UserTokenId_T_TUPLE = 410;
const int UserTokenId_T_FROM = 411;
const int UserTokenId_T_WHERE = 412;
const int UserTokenId_T_JOIN = 413;
const int UserTokenId_T_IN = 414;
const int UserTokenId_T_ON = 415;
const int UserTokenId_T_EQUALS = 416;
const int UserTokenId_T_INTO = 417;
const int UserTokenId_T_LET = 418;
const int UserTokenId_T_ORDERBY = 419;
const int UserTokenId_T_ASCENDING = 420;
const int UserTokenId_T_DESCENDING = 421;
const int UserTokenId_T_SELECT = 422;
const int UserTokenId_T_GROUP = 423;
const int UserTokenId_T_BY = 424;
const int UserTokenId_T_LAMBDA_ARROW = 425;
const int UserTokenId_T_DOUBLE_COLON = 426;
const int UserTokenId_T_LAMBDA_OP = 427;
const int UserTokenId_T_LAMBDA_CP = 428;
const int UserTokenId_T_UNRESOLVED_OP = 429;
const int UserTokenId_T_CALLABLE = 430;
const int UserTokenId_T_ONUMBER = 431;
const int MaxUserTokenId = 432; // Marker, not a real user token ID

#undef YYTOKENTYPE
#undef YYTOKEN_MAP
#undef YYTOKEN
#define YYTOKEN(num, name) UserTokenId_##name,
#define YYTOKEN_MAP static const int user_token_ids[] =
#include "hphp/parser/hphp.tab.hpp"
#undef YYTOKEN_MAP
#undef YYTOKEN

// Converts an internal token ID to a user token ID
static int get_user_token_id(int internal_id) {
  assert(internal_id >= 0);
  if (internal_id < 256) {
    return internal_id;
  }
  if (internal_id >= YYTOKEN_MIN && internal_id <= YYTOKEN_MAX) {
    return user_token_ids[internal_id - YYTOKEN_MIN];
  }
  return MaxUserTokenId;
}

Array f_token_get_all(const String& source) {
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
      Array p = make_packed_array(
        // Convert the internal token ID to a user token ID
        get_user_token_id(tokid),
        String(tok.text()),
        loc.line0
      );
      res.append(p);
    }
  }
  return res;
}

// User token ID => Token name mapping
static const char** getTokenNameTable() {
  static const char* table[MaxUserTokenId+1];
  for (int i = 0; i <= MaxUserTokenId; ++i) {
    table[i] = "UNKNOWN";
  }
#undef YYTOKENTYPE
#undef YYTOKEN_MAP
#undef YYTOKEN
#define YYTOKEN(num, name) table[get_user_token_id(num)] = #name;
#define YYTOKEN_MAP
#include "hphp/parser/hphp.tab.hpp"
#undef YYTOKEN_MAP
#undef YYTOKEN
  return table;
}

// Converts a user token ID to a token name
String f_token_name(int64_t token) {
  static const char** table = getTokenNameTable();
  // For compatibility with parser packages expecting veneration of the
  // lexer's Hebrew roots.
  if (token == k_T_DOUBLE_COLON) {
    return "T_PAAMAYIM_NEKUDOTAYIM";
  }
  if (token >= 0 && token < MaxUserTokenId) {
    return table[token];
  }
  return "UNKNOWN";
}

String f_hphp_to_string(const Variant& v) {
  return v.toString();
}

///////////////////////////////////////////////////////////////////////////////
}

#undef YYTOKENTYPE
#undef YYTOKEN_MAP
#undef YYTOKEN
#define YYTOKEN(num, name)                      \
  extern const int64_t k_##name = get_user_token_id(num);
#define YYTOKEN_MAP namespace HPHP

#include "hphp/parser/hphp.tab.hpp"

namespace HPHP {
extern const int64_t k_T_PAAMAYIM_NEKUDOTAYIM = k_T_DOUBLE_COLON;
}

#undef YYTOKEN_MAP
#undef YYTOKEN
