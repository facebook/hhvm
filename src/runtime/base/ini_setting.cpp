/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/ini_setting.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/hphp_system.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/timeout_thread.h>
#include <runtime/ext/extension.h>
#include <util/lock.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// defined in zend/zend_ini.tab.cpp

extern bool zend_parse_ini_string
(HPHP::CStrRef str, HPHP::CStrRef filename, int scanner_mode,
 HPHP::IniSetting::PFN_PARSER_CALLBACK callback, void *arg);

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool ini_on_update_bool(CStrRef value, void *p) {
  if (p) {
    if ((value.size() == 2 && strcasecmp("on", value.data()) == 0) ||
        (value.size() == 3 && strcasecmp("yes", value.data()) == 0) ||
        (value.size() == 4 && strcasecmp("true", value.data()) == 0)) {
      *((bool*)p) = true;
    } else {
      *((bool*)p) = value.toBoolean();
    }
  }
  return true;
}

bool ini_on_update_long(CStrRef value, void *p) {
  if (p) {
    *((int64*)p) = value.toInt64();
  }
  return true;
}

bool ini_on_update_non_negative(CStrRef value, void *p) {
  int64 v = value.toInt64();
  if (v < 0) {
    return false;
  }
  if (p) {
    *((int64*)p) = v;
  }
  return true;
}

bool ini_on_update_real(CStrRef value, void *p) {
  if (p) {
    *((double*)p) = value.toDouble();
  }
  return true;
}

bool ini_on_update_string(CStrRef value, void *p) {
  if (p) {
    *((std::string*)p) = std::string(value.data(), value.size());
  }
  return true;
}

bool ini_on_update_string_non_empty(CStrRef value, void *p) {
  if (value.empty()) {
    return false;
  }
  if (p) {
    *((std::string*)p) = std::string(value.data(), value.size());
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// callbacks for creating arrays out of ini

static void php_simple_ini_parser_cb
(String *arg1, String *arg2, String *arg3, int callback_type, void *arg) {
  ASSERT(arg1);
  if (!arg1 || !arg2) return;

  Variant *arr = (Variant*)arg;
  switch (callback_type) {
  case IniSetting::ParserEntry:
    arr->set(*arg1, *arg2);
    break;
  case IniSetting::ParserPopEntry:
    {
      Variant &hash = arr->lvalAt(*arg1);
      if (!hash.isArray()) {
        hash = Array::Create();
      }
      if (arg3 && !arg3->empty()) {
        hash.set(*arg3, *arg2);
      } else {
        hash.append(*arg2);
      }
    }
    break;
  }
}

struct CallbackData {
  Variant active_section;
  Variant arr;
};

static void php_ini_parser_cb_with_sections
(String *arg1, String *arg2, String *arg3, int callback_type, void *arg) {
  ASSERT(arg1);
  if (!arg1) return;

  CallbackData *data = (CallbackData*)arg;
  Variant *arr = &data->arr;
  if (callback_type == IniSetting::ParserSection) {
    data->active_section.unset(); // break ref() from previous section
    data->active_section = Array::Create();
    arr->set(*arg1, ref(data->active_section));
  } else if (arg2) {
    Variant *active_arr;
    if (!data->active_section.isNull()) {
      active_arr = &data->active_section;
    } else {
      active_arr = arr;
    }
    php_simple_ini_parser_cb(arg1, arg2, arg3, callback_type, active_arr);
  }
}

///////////////////////////////////////////////////////////////////////////////

static Mutex s_mutex;
Variant IniSetting::FromString(CStrRef ini, CStrRef filename,
                               bool process_sections, int scanner_mode) {
  Lock lock(s_mutex); // ini parser is not thread-safe

  if (process_sections) {
    CallbackData data;
    data.arr = Array::Create();
    if (zend_parse_ini_string
        (ini, filename, scanner_mode, php_ini_parser_cb_with_sections, &data)){
      return data.arr;
    }
  } else {
    Variant ret = Array::Create();
    if (zend_parse_ini_string
        (ini, filename, scanner_mode, php_simple_ini_parser_cb, &ret)) {
      return ret;
    }
  }

  return false;
}

struct UpdateCallbackData {
  IniSetting::PFN_UPDATE_CALLBACK callback;
  void *p;
};

typedef std::map<std::string, UpdateCallbackData> CallbackMap;
static IMPLEMENT_THREAD_LOCAL(CallbackMap, s_callbacks);

typedef std::map<std::string, std::string> DefaultMap;
static DefaultMap s_global_ini;

void IniSetting::SetGlobalDefault(const char *name, const char *value) {
  ASSERT(name && *name);
  ASSERT(value);
  ASSERT(!Extension::ModulesInitialised());

  s_global_ini[name] = value;
}

void IniSetting::Bind(const char *name, const char *value,
                      PFN_UPDATE_CALLBACK callback, void *p /* = NULL */) {
  ASSERT(name && *name);
  ASSERT(value);

  UpdateCallbackData &data = (*s_callbacks)[name];
  data.callback = callback;
  data.p = p;
  (*callback)(value, p);
}

void IniSetting::Unbind(const char *name) {
  ASSERT(name && *name);
  s_callbacks->erase(name);
}

bool IniSetting::Get(CStrRef name, String &value) {
  if (name == "error_reporting") {
    value = String((int64)g_context->getErrorReportingLevel());
    return true;
  }
  if (name == "memory_limit") {
    value = String((int64)g_context->getRequestMemoryMaxBytes());
    return true;
  }
  if (name == "max_execution_time" || name == "maximum_execution_time") {
    value = String((int64)g_context->getRequestTimeLimit());
    return true;
  }
  if (name == "hphp.build_id") {
    value = String(RuntimeOption::BuildId);
    return true;
  }
  if (name == "hphp.compiler_version") {
    value = String(getHphpCompilerVersion());
    return true;
  }
  if (name == "hphp.compiler_id") {
    value = String(getHphpCompilerId());
    return true;
  }
  if (name == "arg_separator.output") {
    value = g_context->getArgSeparatorOutput();
    return true;
  }
  if (name == "upload_max_filesize") {
    int uploadMaxFilesize = VirtualHost::GetUploadMaxFileSize() / (1 << 20);
    value = String(uploadMaxFilesize);
    return true;
  }
  if (name == "log_errors") {
    value = g_context->getLogErrors() ? "1" : "0";
    return true;
  }
  if (name == "error_log") {
    value = g_context->getErrorLog();
    return true;
  }
  if (name == "notice_frequency") {
    value = String((int64)RuntimeOption::NoticeFrequency);
    return true;
  }
  if (name == "warning_frequency") {
    value = String((int64)RuntimeOption::WarningFrequency);
    return true;
  }
  if (name == "include_path") {
    value = g_context->getIncludePath();
    return true;
  }

  DefaultMap::iterator iter = s_global_ini.find(name.data());
  if (iter != s_global_ini.end()) {
    value = iter->second;
    return true;
  }

  return false;
}

bool IniSetting::Set(CStrRef name, CStrRef value) {
  CallbackMap::iterator iter = s_callbacks->find(name.data());
  if (iter != s_callbacks->end()) {
    return (*iter->second.callback)(value, iter->second.p);
  }
  if (name == "memory_limit") {
    if (!value.empty()) {
      int64 newInt = value.toInt64();
      char lastChar = value.charAt(value.size() - 1);
      if (lastChar == 'K' || lastChar == 'k') {
        newInt <<= 10;
      } else if (lastChar == 'M' || lastChar == 'm') {
        newInt <<= 20;
      } else if (lastChar == 'G' || lastChar == 'g') {
        newInt <<= 30;
      }
      g_context->setRequestMemoryMaxBytes(newInt);
      return true;
    }
  } else if (name == "max_execution_time" || name == "maximum_execution_time"){
    int64 limit = value.toInt64();
    TimeoutThread::DeferTimeout(limit);
    // Just for ini_get
    g_context->setRequestTimeLimit(limit);
    return true;
  } else if (name == "arg_separator.output") {
    g_context->setArgSeparatorOutput(value);
    return true;
  } else if (name == "log_errors") {
    bool log;
    ini_on_update_bool(value, &log);
    g_context->setLogErrors(log);
    return true;
  } else if (name == "error_log") {
    g_context->setErrorLog(value);
    return true;
  } else if (name == "notice_frequency") {
    RuntimeOption::NoticeFrequency = value.toInt64();
    return true;
  } else if (name == "warning_frequency") {
    RuntimeOption::WarningFrequency = value.toInt64();
    return true;
  } else if (name == "include_path") {
    g_context->setIncludePath(value);
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
