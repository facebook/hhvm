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

namespace HPHP {
using namespace std;

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
      String parent_class = FrameInjection::GetParentClassName("constant");
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
    const ClassInfo::ConstantInfo *cinfo =
      ClassInfo::FindConstant(name.data());
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
    if (class_exists(className)) { // taking care of volatile class
      const ClassInfo *info;
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

  Eval::Scanner scanner(new ylmm::basic_buffer(iss, false, true),
                        true, false, true);
  Eval::Token tok;
  ylmm::basic_location loc;
  int tokid;
  Array res;
  while ((tokid = scanner.getNextToken(tok, loc))) {
    if (tokid < 256) {
      res.append(String::FromChar((char)tokid));
    } else {
      Array p = CREATE_VECTOR3(tokid, String(tok.getText()), loc.first_line());
      res.append(p);
    }
  }
  return res;
}

#define TOKEN_NAME_ENTRY(number, name, extra) #name

String f_token_name(int64 token) {
  static const char *names[] = {
    TOKEN_NAME_ENTRY(258, T_REQUIRE_ONCE,             RESERVED),
    TOKEN_NAME_ENTRY(259, T_REQUIRE,                  RESERVED),
    TOKEN_NAME_ENTRY(260, T_EVAL,                     RESERVED),
    TOKEN_NAME_ENTRY(261, T_INCLUDE_ONCE,             RESERVED),
    TOKEN_NAME_ENTRY(262, T_INCLUDE,                  RESERVED),
    TOKEN_NAME_ENTRY(263, T_LOGICAL_OR,               RESERVED),
    TOKEN_NAME_ENTRY(264, T_LOGICAL_XOR,              RESERVED),
    TOKEN_NAME_ENTRY(265, T_LOGICAL_AND,              RESERVED),
    TOKEN_NAME_ENTRY(266, T_PRINT,                    RESERVED),
    TOKEN_NAME_ENTRY(267, T_SR_EQUAL,                 RESERVED),
    TOKEN_NAME_ENTRY(268, T_SL_EQUAL,                 RESERVED),
    TOKEN_NAME_ENTRY(269, T_XOR_EQUAL,                RESERVED),
    TOKEN_NAME_ENTRY(270, T_OR_EQUAL,                 RESERVED),
    TOKEN_NAME_ENTRY(271, T_AND_EQUAL,                RESERVED),
    TOKEN_NAME_ENTRY(272, T_MOD_EQUAL,                RESERVED),
    TOKEN_NAME_ENTRY(273, T_CONCAT_EQUAL,             RESERVED),
    TOKEN_NAME_ENTRY(274, T_DIV_EQUAL,                RESERVED),
    TOKEN_NAME_ENTRY(275, T_MUL_EQUAL,                RESERVED),
    TOKEN_NAME_ENTRY(276, T_MINUS_EQUAL,              RESERVED),
    TOKEN_NAME_ENTRY(277, T_PLUS_EQUAL,               RESERVED),
    TOKEN_NAME_ENTRY(278, T_BOOLEAN_OR,               RESERVED),
    TOKEN_NAME_ENTRY(279, T_BOOLEAN_AND,              RESERVED),
    TOKEN_NAME_ENTRY(280, T_IS_NOT_IDENTICAL,         RESERVED),
    TOKEN_NAME_ENTRY(281, T_IS_IDENTICAL,             RESERVED),
    TOKEN_NAME_ENTRY(282, T_IS_NOT_EQUAL,             RESERVED),
    TOKEN_NAME_ENTRY(283, T_IS_EQUAL,                 RESERVED),
    TOKEN_NAME_ENTRY(284, T_IS_GREATER_OR_EQUAL,      RESERVED),
    TOKEN_NAME_ENTRY(285, T_IS_SMALLER_OR_EQUAL,      RESERVED),
    TOKEN_NAME_ENTRY(286, T_SR,                       RESERVED),
    TOKEN_NAME_ENTRY(287, T_SL,                       RESERVED),
    TOKEN_NAME_ENTRY(288, T_INSTANCEOF,               RESERVED),
    TOKEN_NAME_ENTRY(289, T_UNSET_CAST,               RESERVED),
    TOKEN_NAME_ENTRY(290, T_BOOL_CAST,                RESERVED),
    TOKEN_NAME_ENTRY(291, T_OBJECT_CAST,              RESERVED),
    TOKEN_NAME_ENTRY(292, T_ARRAY_CAST,               RESERVED),
    TOKEN_NAME_ENTRY(293, T_STRING_CAST,              RESERVED),
    TOKEN_NAME_ENTRY(294, T_DOUBLE_CAST,              RESERVED),
    TOKEN_NAME_ENTRY(295, T_INT_CAST,                 RESERVED),
    TOKEN_NAME_ENTRY(296, T_DEC,                      RESERVED),
    TOKEN_NAME_ENTRY(297, T_INC,                      RESERVED),
    TOKEN_NAME_ENTRY(298, T_CLONE,                    RESERVED),
    TOKEN_NAME_ENTRY(299, T_NEW,                      RESERVED),
    TOKEN_NAME_ENTRY(300, T_EXIT,                     RESERVED),
    TOKEN_NAME_ENTRY(301, T_IF,                       RESERVED),
    TOKEN_NAME_ENTRY(302, T_ELSEIF,                   RESERVED),
    TOKEN_NAME_ENTRY(303, T_ELSE,                     RESERVED),
    TOKEN_NAME_ENTRY(304, T_ENDIF,                    RESERVED),
    TOKEN_NAME_ENTRY(305, T_LNUMBER,                  RESERVED),
    TOKEN_NAME_ENTRY(306, T_DNUMBER,                  RESERVED),
    TOKEN_NAME_ENTRY(307, T_STRING,                   RESERVED),
    TOKEN_NAME_ENTRY(308, T_STRING_VARNAME,           RESERVED),
    TOKEN_NAME_ENTRY(309, T_VARIABLE,                 RESERVED),
    TOKEN_NAME_ENTRY(310, T_NUM_STRING,               RESERVED),
    TOKEN_NAME_ENTRY(311, T_INLINE_HTML,              RESERVED),
    TOKEN_NAME_ENTRY(312, T_CHARACTER,                RESERVED),
    TOKEN_NAME_ENTRY(313, T_BAD_CHARACTER,            RESERVED),
    TOKEN_NAME_ENTRY(314, T_ENCAPSED_AND_WHITESPACE,  RESERVED),
    TOKEN_NAME_ENTRY(315, T_CONSTANT_ENCAPSED_STRING, RESERVED),
    TOKEN_NAME_ENTRY(316, T_ECHO,                     RESERVED),
    TOKEN_NAME_ENTRY(317, T_DO,                       RESERVED),
    TOKEN_NAME_ENTRY(318, T_WHILE,                    RESERVED),
    TOKEN_NAME_ENTRY(319, T_ENDWHILE,                 RESERVED),
    TOKEN_NAME_ENTRY(320, T_FOR,                      RESERVED),
    TOKEN_NAME_ENTRY(321, T_ENDFOR,                   RESERVED),
    TOKEN_NAME_ENTRY(322, T_FOREACH,                  RESERVED),
    TOKEN_NAME_ENTRY(323, T_ENDFOREACH,               RESERVED),
    TOKEN_NAME_ENTRY(324, T_DECLARE,                  RESERVED),
    TOKEN_NAME_ENTRY(325, T_ENDDECLARE,               RESERVED),
    TOKEN_NAME_ENTRY(326, T_AS,                       RESERVED),
    TOKEN_NAME_ENTRY(327, T_SWITCH,                   RESERVED),
    TOKEN_NAME_ENTRY(328, T_ENDSWITCH,                RESERVED),
    TOKEN_NAME_ENTRY(329, T_CASE,                     RESERVED),
    TOKEN_NAME_ENTRY(330, T_DEFAULT,                  RESERVED),
    TOKEN_NAME_ENTRY(331, T_BREAK,                    RESERVED),
    TOKEN_NAME_ENTRY(332, T_CONTINUE,                 RESERVED),
    TOKEN_NAME_ENTRY(333, T_FUNCTION,                 RESERVED),
    TOKEN_NAME_ENTRY(334, T_CONST,                    RESERVED),
    TOKEN_NAME_ENTRY(335, T_RETURN,                   RESERVED),
    TOKEN_NAME_ENTRY(336, T_TRY,                      RESERVED),
    TOKEN_NAME_ENTRY(337, T_CATCH,                    RESERVED),
    TOKEN_NAME_ENTRY(338, T_THROW,                    RESERVED),
    TOKEN_NAME_ENTRY(339, T_USE,                      RESERVED),
    TOKEN_NAME_ENTRY(340, T_GLOBAL,                   RESERVED),
    TOKEN_NAME_ENTRY(341, T_PUBLIC,                   RESERVED),
    TOKEN_NAME_ENTRY(342, T_PROTECTED,                RESERVED),
    TOKEN_NAME_ENTRY(343, T_PRIVATE,                  RESERVED),
    TOKEN_NAME_ENTRY(344, T_FINAL,                    RESERVED),
    TOKEN_NAME_ENTRY(345, T_ABSTRACT,                 RESERVED),
    TOKEN_NAME_ENTRY(346, T_STATIC,                   RESERVED),
    TOKEN_NAME_ENTRY(347, T_VAR,                      RESERVED),
    TOKEN_NAME_ENTRY(348, T_UNSET,                    RESERVED),
    TOKEN_NAME_ENTRY(349, T_ISSET,                    RESERVED),
    TOKEN_NAME_ENTRY(350, T_EMPTY,                    RESERVED),
    TOKEN_NAME_ENTRY(351, T_HALT_COMPILER,            RESERVED),
    TOKEN_NAME_ENTRY(352, T_CLASS,                    RESERVED),
    TOKEN_NAME_ENTRY(353, T_INTERFACE,                RESERVED),
    TOKEN_NAME_ENTRY(354, T_EXTENDS,                  RESERVED),
    TOKEN_NAME_ENTRY(355, T_IMPLEMENTS,               RESERVED),
    TOKEN_NAME_ENTRY(356, T_OBJECT_OPERATOR,          RESERVED),
    TOKEN_NAME_ENTRY(357, T_DOUBLE_ARROW,             RESERVED),
    TOKEN_NAME_ENTRY(358, T_LIST,                     RESERVED),
    TOKEN_NAME_ENTRY(359, T_ARRAY,                    RESERVED),
    TOKEN_NAME_ENTRY(360, T_CLASS_C,                  RESERVED),
    TOKEN_NAME_ENTRY(361, T_METHOD_C,                 RESERVED),
    TOKEN_NAME_ENTRY(362, T_FUNC_C,                   RESERVED),
    TOKEN_NAME_ENTRY(363, T_LINE,                     RESERVED),
    TOKEN_NAME_ENTRY(364, T_FILE,                     RESERVED),
    TOKEN_NAME_ENTRY(365, T_COMMENT,                  RESERVED),
    TOKEN_NAME_ENTRY(366, T_DOC_COMMENT,              RESERVED),
    TOKEN_NAME_ENTRY(367, T_OPEN_TAG,                 RESERVED),
    TOKEN_NAME_ENTRY(368, T_OPEN_TAG_WITH_ECHO,       RESERVED),
    TOKEN_NAME_ENTRY(369, T_CLOSE_TAG,                RESERVED),
    TOKEN_NAME_ENTRY(370, T_WHITESPACE,               RESERVED),
    TOKEN_NAME_ENTRY(371, T_START_HEREDOC,            RESERVED),
    TOKEN_NAME_ENTRY(372, T_END_HEREDOC,              RESERVED),
    TOKEN_NAME_ENTRY(373, T_DOLLAR_OPEN_CURLY_BRACES, RESERVED),
    TOKEN_NAME_ENTRY(374, T_CURLY_OPEN,               RESERVED),
    TOKEN_NAME_ENTRY(375, T_PAAMAYIM_NEKUDOTAYIM,     RESERVED),
  };

  if (token >= 258 && token <= 375) {
    return names[token - 258];
  }
  return "UNKNOWN";
}

///////////////////////////////////////////////////////////////////////////////
}
