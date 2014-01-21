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

#define PHP_INI_USER 1
#define PHP_INI_PERDIR (1<<1)
#define PHP_INI_SYSTEM (1<<2)
#define PHP_INI_ALL (PHP_INI_USER|PHP_INI_PERDIR|PHP_INI_SYSTEM)

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const Extension* IniSetting::CORE = (Extension*)(-1);

const StaticString
  s_1("1"),
  s_0("0"),
  s_global_value("global_value"),
  s_local_value("local_value"),
  s_access("access"),
  s_core("core");

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

bool ini_on_update_int(const String& value, void *p) {
  if (p) {
    *((int*)p) = value.toInt32();
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

bool ini_on_update_stdstring(const String& value, void *p) {
  if (p) {
    *((std::string*)p) = std::string(value.data(), value.size());
  }
  return true;
}

bool ini_on_update_string(const String& value, void *p) {
  if (p) {
    *((String*)p) = value;
  }
  return true;
}

String ini_get_bool(void *p) {
  return *(bool*) p;
}

String ini_get_bool_as_int(void* p) {
  if (*((bool*)p)) {
    return s_1;
  }
  return s_0;
}

String ini_get_int(void *p) {
  return *((int*)p);
}

String ini_get_long(void *p) {
  return *((int64_t*)p);
}

String ini_get_real(void *p) {
  return *((double*)p);
}

String ini_get_string(void *p) {
  return *((String*)p);
}

String ini_get_stdstring(void *p) {
  return *((std::string*)p);
}

String ini_get_static_string_1(void* p) {
  return s_1;
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
  const Extension* extension;
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

void IniSetting::Bind(const Extension* extension,
                      const char *name, const char *value,
                      UpdateCallback updateCallback, GetCallback getCallback,
                      void *p /* = NULL */) {
  assert(value);

  Bind(extension, name, updateCallback, getCallback, p);

  updateCallback(value, p);
}

void IniSetting::Bind(const Extension* extension,
                      const char *name,
                      UpdateCallback updateCallback, GetCallback getCallback,
                      void *p /* = NULL */) {
  assert(name && *name);

  auto &data = (*s_callbacks)[name];
  data.extension = extension;
  data.updateCallback = updateCallback;
  data.getCallback = getCallback;
  data.p = p;
}

void IniSetting::Unbind(const char *name) {
  assert(name && *name);
  s_callbacks->erase(name);
}

bool IniSetting::Get(const String& name, String &value) {
  DefaultMap::iterator iter = s_global_ini.find(name.data());
  if (iter != s_global_ini.end()) {
    value = iter->second;
    return true;
  }

  CallbackMap::iterator cb_iter = s_callbacks->find(name.data());
  if (cb_iter != s_callbacks->end()) {
    value = cb_iter->second.getCallback(cb_iter->second.p);
    return true;
  }

  return false;
}

bool IniSetting::Set(const String& name, const String& value) {
  CallbackMap::iterator iter = s_callbacks->find(name.data());
  if (iter != s_callbacks->end()) {
    if (iter->second.updateCallback) {
      return iter->second.updateCallback(value, iter->second.p);
    }
  }

  return false;
}

Array IniSetting::GetAll(const String& ext_name, bool details) {
  Array r = Array::Create();

  const Extension* ext = nullptr;
  if (!ext_name.empty()) {
    if (ext_name == s_core) {
      ext = IniSetting::CORE;
    } else {
      ext = Extension::GetExtension(ext_name);
      if (!ext) {
        raise_warning("Unable to find extension '%s'",
                      ext_name.toCppString().c_str());
        return r;
      }
    }
  }

  for (auto& iter: (*s_callbacks)) {
    if (ext && ext != iter.second.extension) {
      continue;
    }

    if (details) {
      Array item = Array::Create();
      String value(iter.second.getCallback(iter.second.p));
      item.add(s_global_value, value);
      item.add(s_local_value, value);
      // HHVM doesn't support varying access levels, but we can at least
      // indicate if ini_set() should work
      if (iter.second.updateCallback) {
        item.add(s_access, Variant(PHP_INI_ALL));
      } else {
        item.add(s_access, Variant(PHP_INI_SYSTEM | PHP_INI_PERDIR));
      }
      r.add(String(iter.first), item);
    } else {
      r.add(String(iter.first), iter.second.getCallback(iter.second.p));
    }
  }
  return r;
}


///////////////////////////////////////////////////////////////////////////////
}
