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

#include "hphp/runtime/ext/std/ext_std_misc.h"
#include <limits>

#include "hphp/parser/scanner.h"
#include "hphp/runtime/base/actrec-args.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/zend-pack.h"
#include "hphp/runtime/ext/std/ext_std_math.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/system/constants.h"
#include "hphp/util/logger.h"
#include <sys/param.h> // MAXPATHLEN is here
#include "hphp/runtime/base/comparisons.h"

namespace HPHP {

IMPLEMENT_THREAD_LOCAL(std::string, s_misc_highlight_default_string);
IMPLEMENT_THREAD_LOCAL(std::string, s_misc_highlight_default_comment);
IMPLEMENT_THREAD_LOCAL(std::string, s_misc_highlight_default_keyword);
IMPLEMENT_THREAD_LOCAL(std::string, s_misc_highlight_default_default);
IMPLEMENT_THREAD_LOCAL(std::string, s_misc_highlight_default_html);
IMPLEMENT_THREAD_LOCAL(std::string, s_misc_display_errors);

const std::string s_1("1"), s_2("2"), s_stdout("stdout"), s_stderr("stderr");
const double k_INF = std::numeric_limits<double>::infinity();
const double k_NAN = std::numeric_limits<double>::quiet_NaN();

static String HHVM_FUNCTION(server_warmup_status) {
  // Fail if we jitted more than 25kb of code.
  size_t begin, end;
  jit::mcg->codeEmittedThisRequest(begin, end);
  auto const diff = end - begin;
  auto constexpr kMaxTCBytes = 25 << 10;
  if (diff > kMaxTCBytes) {
    return folly::format("Translation cache grew by {} bytes to {} bytes.",
                         diff, begin).str();
  }

  // Fail if we spent more than 0.5ms in the JIT.
  auto const jittime = jit::Timer::CounterValue(jit::Timer::translate);
  auto constexpr kMaxJitTimeNS = 500000;
  if (jittime.total > kMaxJitTimeNS) {
    return folly::format("Spent {}us in the JIT.", jittime.total / 1000).str();
  }

  if (!isStandardRequest()) {
    return "Warmup is still in progress.";
  }

  if (requestCount() <= RuntimeOption::EvalJitProfileRequests) {
    return "PGO profiling translations are still enabled.";
  }

  auto tpc_diff = jit::s_perfCounters[jit::tpc_interp_bb] -
    jit::s_perfCounters[jit::tpc_interp_bb_force];
  if (tpc_diff) {
    return folly::sformat("Interpreted {} non-forced basic blocks.", tpc_diff);
  }

  return empty_string();
}


void StandardExtension::threadInitMisc() {
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
      "display_errors", RuntimeOption::EnableHipHopSyntax ? "stderr" : "1",
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

static void bindTokenConstants();
static int get_user_token_id(int internal_id);
const StaticString s_T_PAAMAYIM_NEKUDOTAYIM("T_PAAMAYIM_NEKUDOTAYIM");

void StandardExtension::initMisc() {
    HHVM_FALIAS(HH\\server_warmup_status, server_warmup_status);
    HHVM_FE(connection_aborted);
    HHVM_FE(connection_status);
    HHVM_FE(connection_timeout);
    HHVM_FE(constant);
    HHVM_FE(define);
    HHVM_FE(defined);
    HHVM_FE(ignore_user_abort);
    HHVM_FE(pack);
    HHVM_FE(sleep);
    HHVM_FE(usleep);
    HHVM_FE(time_nanosleep);
    HHVM_FE(time_sleep_until);
    HHVM_FE(uniqid);
    HHVM_FE(unpack);
    HHVM_FE(sys_getloadavg);
    HHVM_FE(token_get_all);
    HHVM_FE(token_name);
    HHVM_FE(hphp_to_string);
    HHVM_FALIAS(__SystemLib\\max2, SystemLib_max2);
    HHVM_FALIAS(__SystemLib\\min2, SystemLib_min2);
    Native::registerConstant<KindOfDouble>(makeStaticString("INF"), k_INF);
    Native::registerConstant<KindOfDouble>(makeStaticString("NAN"), k_NAN);
    Native::registerConstant<KindOfInt64>(
        makeStaticString("PHP_MAXPATHLEN"), MAXPATHLEN);
    Native::registerConstant<KindOfBoolean>(makeStaticString("PHP_DEBUG"),
      #if DEBUG
        true
      #else
        false
      #endif
     );
    bindTokenConstants();
    Native::registerConstant<KindOfInt64>(s_T_PAAMAYIM_NEKUDOTAYIM.get(),
                                          get_user_token_id(T_DOUBLE_COLON));

    loadSystemlib("std_misc");
  }

// Make sure "tokenizer" gets added to the list of extensions
IMPLEMENT_DEFAULT_EXTENSION_VERSION(tokenizer, 0.1);


///////////////////////////////////////////////////////////////////////////////

int64_t HHVM_FUNCTION(connection_aborted) {
  return HHVM_FN(connection_status)() == k_CONNECTION_ABORTED;
}

int64_t HHVM_FUNCTION(connection_status) {
  return k_CONNECTION_NORMAL;
}

int64_t HHVM_FUNCTION(connection_timeout) {
  return HHVM_FN(connection_status)() == k_CONNECTION_TIMEOUT;
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

Variant HHVM_FUNCTION(constant, const String& name) {
  if (!name.get()) return init_null();
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
    auto const cns = Unit::loadCns(name.get());
    if (cns) return tvAsCVarRef(cns);
  }

  return init_null();
}

bool HHVM_FUNCTION(define, const String& name, const Variant& value,
              bool case_insensitive /* = false */) {
  if (case_insensitive) {
    raise_warning(Strings::CONSTANTS_CASE_SENSITIVE);
  }
  return Unit::defCns(name.get(), value.asCell());
}

bool HHVM_FUNCTION(defined, const String& name, bool autoload /* = true */) {
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

int64_t HHVM_FUNCTION(ignore_user_abort, bool setting /* = false */) {
  return 0;
}

TypedValue* HHVM_FUNCTION(pack, ActRec* ar) {
  int num = ar->numArgs();
  String format(getArg<KindOfString>(ar,0));
  Array extra = Array::Create();
  for (int i = 1; i<num; i++) {
    extra.append(getArgVariant(ar,i));
  }
  Variant result = ZendPack().pack(format, extra);
  // pack() returns false if there was an error
  if (!result.isBoolean()) {
    tvCastToStringInPlace(result.asTypedValue());
  }
  return arReturn(ar, std::move(result));
}

int64_t HHVM_FUNCTION(sleep, int seconds) {
  IOStatusHelper io("sleep");
  Transport *transport = g_context->getTransport();
  if (transport) {
    transport->incSleepTime(seconds);
  }
  sleep(seconds);
  return 0;
}

void HHVM_FUNCTION(usleep, int micro_seconds) {
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

Variant HHVM_FUNCTION(time_nanosleep, int seconds, int nanoseconds) {
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

bool HHVM_FUNCTION(time_sleep_until, double timestamp) {
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

String HHVM_FUNCTION(uniqid, const String& prefix /* = null_string */,
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
  auto ptr = uniqid.mutableData();
  // StringData::capacity() returns the buffer size without the null
  // terminator. snprintf expects a the buffer capacity including room
  // for the null terminator, writes the null termintor, and returns
  // the full length not counting the null terminator.
  auto capacity = uniqid.capacity() + 1;
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

Variant HHVM_FUNCTION(unpack, const String& format, const String& data) {
  return ZendPack().unpack(format, data);
}

Array HHVM_FUNCTION(sys_getloadavg) {
#if (defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER))
  return make_packed_array(0, 0, 0);
#else
  double load[3];
  getloadavg(load, 3);
  return make_packed_array(load[0], load[1], load[2]);
#endif
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
const int UserTokenId_T_PAAMAYIM_NEKUDOTAYIM UNUSED = 376;
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
const int UserTokenId_T_ENUM = 388;
const int UserTokenId_T_XHP_REQUIRED = 389;
const int UserTokenId_T_TRAIT = 390;
const int UserTokenId_T_INSTEADOF = 391;
const int UserTokenId_T_TRAIT_C = 392;
const int UserTokenId_T_ELLIPSIS = 393;
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
const int UserTokenId_T_POW = 432;
const int UserTokenId_T_POW_EQUAL = 433;
const int UserTokenId_T_NULLSAFE_OBJECT_OPERATOR = 434;
const int UserTokenId_T_HASHBANG = 435;
const int UserTokenId_T_SUPER = 436;
const int MaxUserTokenId = 437; // Marker, not a real user token ID

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

/**
 * We cheat slightly in the lexer by turning
 * T_ELSE T_WHITESPACE T_IF into T_ELSEIF
 *
 * This makes the AST flatter and avoids bugs like
 * https://github.com/facebook/hhvm/issues/2699
 */
static String token_get_all_fix_elseif(Array& res,
                                       int& tokVal,
                                       const std::string& tokText,
                                       Location& loc) {
  if (!strcasecmp(tokText.c_str(), "elseif")) {
    // Actual T_ELSEIF, continue on.
    return String(tokText);
  }

  // Otherwise, it's a fake elseif made from "else\s+if"
  auto tokCStr = tokText.c_str();
  auto tokCEnd = tokCStr + tokText.size();

  const auto DEBUG_ONLY checkWhitespace =
  [](const char* s, const char* e) {
    while (s < e) { if (!isspace(*(s++))) return false; }
    return true;
  };

  assert(tokText.size() > strlen("elseif"));
  assert(!strncasecmp(tokCStr, "else", strlen("else")));
  assert(checkWhitespace(tokCStr + strlen("else"), tokCEnd - strlen("if")));
  assert(!strcasecmp(tokCEnd - strlen("if"), "if"));

  // Shove in the T_ELSE and T_WHITESPACE, then return the remaining T_IF
  res.append(make_packed_array(
    UserTokenId_T_ELSE,
    String(tokCStr, strlen("else"), CopyString),
    loc.r.line0
  ));

  res.append(make_packed_array(
    UserTokenId_T_WHITESPACE,
    String(tokCStr + strlen("else"), tokText.size() - strlen("elseif"),
           CopyString),
    loc.r.line0
  ));

  tokVal = UserTokenId_T_IF;

  // To account for newlines in the T_WHITESPACE
  loc.r.line0 = loc.r.line1;

  return String(tokCEnd - strlen("if"), CopyString);
}

Array HHVM_FUNCTION(token_get_all, const String& source) {
  Scanner scanner(source.data(), source.size(),
                  RuntimeOption::GetScannerType() | Scanner::ReturnAllTokens);
  ScannerToken tok;
  Location loc;
  int tokid;
  Array res = Array::Create();
  while ((tokid = scanner.getNextToken(tok, loc))) {
loop_start: // For after seeing a T_INLINE_HTML, see below
    if (tokid < 256) {
      res.append(String::FromChar((char)tokid));
    } else {
      String value;
      int tokVal = get_user_token_id(tokid);
      switch (tokVal) {
        case UserTokenId_T_XHP_LABEL:
          value = String(":" + tok.text());
          break;
        case UserTokenId_T_XHP_CATEGORY_LABEL:
          value = String("%" + tok.text());
          break;
        case UserTokenId_T_ELSEIF:
          value = token_get_all_fix_elseif(res, tokVal, tok.text(), loc);
          break;
        case UserTokenId_T_HASHBANG:
          // Convert T_HASHBANG to T_INLINE_HTML for Zend compatibility
          tokVal = UserTokenId_T_INLINE_HTML;
          // Fall through to merge it with following T_INLINE_HTML tokens
        case UserTokenId_T_INLINE_HTML:
        {
          // Consecutive T_INLINE_HTML tokens should be merged together to
          // match Zend behaviour.
          value = String(tok.text());
          int line = loc.r.line0;
          tokid = scanner.getNextToken(tok, loc);
          while (tokid == T_INLINE_HTML) {
            value += String(tok.text());
            tokid = scanner.getNextToken(tok, loc);
          }
          Array p = make_packed_array(
            tokVal,
            value,
            line
          );
          res.append(p);

          if (tokid) {
            // We have a new token to deal with, jump to the beginning
            // of the loop, but don't fetch the next token, hence the
            // goto.
            goto loop_start;
          } else {
            // Break out otherwise we end up appending an empty token to
            // the end of the array
            return res;
          }
          break;
        }
        default:
          value = String(tok.text());
          break;
      }

      Array p = make_packed_array(
        tokVal,
        value,
        loc.r.line0
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
#include "hphp/parser/hphp.tab.hpp" // nolint
#undef YYTOKEN_MAP
#undef YYTOKEN
  return table;
}

// Converts a user token ID to a token name
String HHVM_FUNCTION(token_name, int64_t token) {
  static const char** table = getTokenNameTable();
  if (token >= 0 && token < MaxUserTokenId) {
    return table[token];
  }
  return "UNKNOWN";
}

String HHVM_FUNCTION(hphp_to_string, const Variant& v) {
  return v.toString();
}

// Adds an optimized FCallBuiltin for max with 2 operands to SystemLib
Variant HHVM_FUNCTION(SystemLib_max2, const Variant& value1,
                      const Variant& value2) {
  return less(value1, value2) ? value2 : value1;
}

// Adds an optimized FCallBuiltin for min with 2 operands to SystemLib
Variant HHVM_FUNCTION(SystemLib_min2, const Variant& value1,
                      const Variant& value2) {
  return more(value1, value2) ? value2 : value1;
}

#undef YYTOKENTYPE
#undef YYTOKEN_MAP
#undef YYTOKEN
#define YYTOKEN_MAP static void bindTokenConstants()
#define YYTOKEN(num, name) Native::registerConstant<KindOfInt64> \
  (makeStaticString(#name), get_user_token_id(num));

#include "hphp/parser/hphp.tab.hpp" // nolint

#undef YYTOKEN_MAP
#undef YYTOKEN

///////////////////////////////////////////////////////////////////////////////
}
