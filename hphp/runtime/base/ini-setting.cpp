/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/ini-setting.h"

#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/hphp-system.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/ini-parser/zend-ini.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/util/lock.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool ini_on_update_bool(const String& value, void *p) {
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

bool ini_on_update_long(const String& value, void *p) {
  if (p) {
    *((int64_t*)p) = value.toInt64();
  }
  return true;
}

bool ini_on_update_non_negative(const String& value, void *p) {
  int64_t v = value.toInt64();
  if (v < 0) {
    return false;
  }
  if (p) {
    *((int64_t*)p) = v;
  }
  return true;
}

bool ini_on_update_real(const String& value, void *p) {
  if (p) {
    *((double*)p) = value.toDouble();
  }
  return true;
}

bool ini_on_update_string(const String& value, void *p) {
  if (p) {
    *((std::string*)p) = std::string(value.data(), value.size());
  }
  return true;
}

bool ini_on_update_string_non_empty(const String& value, void *p) {
  if (value.empty()) {
    return false;
  }
  if (p) {
    *((std::string*)p) = std::string(value.data(), value.size());
  }
  return true;
}

String ini_get_bool(void *p) {
  return *(bool*) p;
}

String ini_get_long(void *p) {
  return *((int64_t*)p);
}

String ini_get_real(void *p) {
  return *((double*)p);
}

String ini_get_string(void *p) {
  return *((std::string*)p);
}

///////////////////////////////////////////////////////////////////////////////
// callbacks for creating arrays out of ini

static void php_simple_ini_parser_cb
(String *arg1, String *arg2, String *arg3, int callback_type, void *arg) {
  assert(arg1);
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
  assert(arg1);
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
Variant IniSetting::FromString(const String& ini, const String& filename,
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

struct IniCallbackData {
  IniSetting::UpdateCallback updateCallback;
  IniSetting::GetCallback getCallback;
  void *p;
};

typedef std::map<std::string, IniCallbackData> CallbackMap;
static IMPLEMENT_THREAD_LOCAL(CallbackMap, s_callbacks);

typedef std::map<std::string, std::string> DefaultMap;
static DefaultMap s_global_ini;

void IniSetting::SetGlobalDefault(const char *name, const char *value) {
  assert(name && *name);
  assert(value);
  assert(!Extension::ModulesInitialised());

  s_global_ini[name] = value;
}

void IniSetting::Bind(const char *name, const char *value,
                      UpdateCallback updateCallback, GetCallback getCallback,
                      void *p /* = NULL */) {
  assert(name && *name);
  assert(value);

  auto &data = (*s_callbacks)[name];
  data.updateCallback = updateCallback;
  data.getCallback = getCallback;
  data.p = p;
  (*updateCallback)(value, p);
}

void IniSetting::Unbind(const char *name) {
  assert(name && *name);
  s_callbacks->erase(name);
}

const StaticString
  s_allow_url_fopen("allow_url_fopen"),
  s_error_reporting("error_reporting"),
  s_memory_limit("memory_limit"),
  s_max_execution_time("max_execution_time"),
  s_maximum_execution_time("maximum_execution_time"),
  s_hphp_build_id("hphp.build_id"),
  s_hphp_compiler_version("hphp.compiler_version"),
  s_hphp_compiler_id("hphp.compiler_id"),
  s_arg_separator_output("arg_separator.output"),
  s_file_uploads("file_uploads"),
  s_upload_tmp_dir("upload_tmp_dir"),
  s_upload_max_filesize("upload_max_filesize"),
  s_post_max_size("post_max_size"),
  s_log_errors("log_errors"),
  s_error_log("error_log"),
  s_notice_frequency("notice_frequency"),
  s_warning_frequency("warning_frequency"),
  s_include_path("include_path"),
  s_1("1"),
  s_0("0");

bool IniSetting::Get(const String& name, String &value) {
  if (name == s_error_reporting) {
    value = String((int64_t)g_context->getErrorReportingLevel());
    return true;
  }
  if (name == s_memory_limit) {
    value = g_context->getRequestMemoryMaxBytes();
    return true;
  }
  if (name == s_max_execution_time || name == s_maximum_execution_time) {
    int64_t timeout = ThreadInfo::s_threadInfo.getNoCheck()->
      m_reqInjectionData.getTimeout();
    value = String(timeout);
    return true;
  }
  if (name == s_hphp_build_id) {
    value = String(RuntimeOption::BuildId);
    return true;
  }
  if (name == s_hphp_compiler_version) {
    value = String(getHphpCompilerVersion());
    return true;
  }
  if (name == s_hphp_compiler_id) {
    value = String(getHphpCompilerId());
    return true;
  }
  if (name == s_arg_separator_output) {
    value = g_context->getArgSeparatorOutput();
    return true;
  }
  if (name == s_file_uploads) {
    value = RuntimeOption::EnableFileUploads ? s_1 : s_0;
    return true;
  }
  if (name == s_upload_tmp_dir) {
    value = String(RuntimeOption::UploadTmpDir);
    return true;
  }
  if (name == s_upload_max_filesize) {
    int uploadMaxFilesize = VirtualHost::GetUploadMaxFileSize() / (1 << 20);
    value = String(uploadMaxFilesize) + "M";
    return true;
  }
  if (name == s_post_max_size) {
    int postMaxSize = VirtualHost::GetMaxPostSize();
    value = String(postMaxSize);
    return true;
  }
  if (name == s_log_errors) {
    value = g_context->getLogErrors() ? s_1 : s_0;
    return true;
  }
  if (name == s_error_log) {
    value = g_context->getErrorLog();
    return true;
  }
  if (name == s_notice_frequency) {
    value = String((int64_t)RuntimeOption::NoticeFrequency);
    return true;
  }
  if (name == s_warning_frequency) {
    value = String((int64_t)RuntimeOption::WarningFrequency);
    return true;
  }
  if (name == s_include_path) {
    value = g_context->getIncludePath();
    return true;
  }
  if (name == s_allow_url_fopen) {
    value = s_1;
    return true;
  }

  DefaultMap::iterator iter = s_global_ini.find(name.data());
  if (iter != s_global_ini.end()) {
    value = iter->second;
    return true;
  }

  CallbackMap::iterator cb_iter = s_callbacks->find(name.data());
  if (cb_iter != s_callbacks->end()) {
    value = (*cb_iter->second.getCallback)(cb_iter->second.p);
    return true;
  }

  return false;
}

bool IniSetting::Set(const String& name, const String& value) {
  CallbackMap::iterator iter = s_callbacks->find(name.data());
  if (iter != s_callbacks->end()) {
    return (*iter->second.updateCallback)(value, iter->second.p);
  }

  if (name == s_error_reporting) {
    g_context->setErrorReportingLevel(value.toInt64());
    return true;
  } else if (name == s_memory_limit) {
    if (!value.empty()) {
      g_context->setRequestMemoryMaxBytes(value);
      return true;
    }
  } else if (name == s_max_execution_time || name == s_maximum_execution_time) {
    int64_t limit = value.toInt64();
    ThreadInfo::s_threadInfo.getNoCheck()->
      m_reqInjectionData.setTimeout(limit);
    return true;
  } else if (name == s_arg_separator_output) {
    g_context->setArgSeparatorOutput(value);
    return true;
  } else if (name == s_log_errors) {
    bool log;
    ini_on_update_bool(value, &log);
    g_context->setLogErrors(log);
    return true;
  } else if (name == s_error_log) {
    g_context->setErrorLog(value);
    return true;
  } else if (name == s_notice_frequency) {
    RuntimeOption::NoticeFrequency = value.toInt64();
    return true;
  } else if (name == s_warning_frequency) {
    RuntimeOption::WarningFrequency = value.toInt64();
    return true;
  } else if (name == s_include_path) {
    g_context->setIncludePath(value);
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
