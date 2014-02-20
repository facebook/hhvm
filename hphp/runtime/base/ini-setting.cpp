/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/zend-strtod.h"
#include "hphp/runtime/ext/ext_misc.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/util/lock.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const Extension* IniSetting::CORE = (Extension*)(-1);

const StaticString
  s_global_value("global_value"),
  s_local_value("local_value"),
  s_access("access"),
  s_core("core");

int64_t convert_bytes_to_long(const std::string& value) {
  int64_t newInt = strtoll(value.c_str(), nullptr, 10);
  char lastChar = value.at(value.size() - 1);
  if (lastChar == 'K' || lastChar == 'k') {
    newInt <<= 10;
  } else if (lastChar == 'M' || lastChar == 'm') {
    newInt <<= 20;
  } else if (lastChar == 'G' || lastChar == 'g') {
    newInt <<= 30;
  }
  return newInt;
}

bool ini_on_update_bool(const std::string& value, void *p) {
  if (p) {
    if ((value.size() == 0) ||
        (value.size() == 1 && value == "0") ||
        (value.size() == 2 && strcasecmp("no", value.data()) == 0) ||
        (value.size() == 3 && strcasecmp("off", value.data()) == 0) ||
        (value.size() == 5 && strcasecmp("false", value.data()) == 0)) {
      *((bool*)p) = false;
    } else {
      *((bool*)p) = true;
    }
  }
  return true;
}

bool ini_on_update_long(const std::string& value, void *p) {
  if (p) {
    *((int64_t*)p) = convert_bytes_to_long(value);
  }
  return true;
}

bool ini_on_update_non_negative(const std::string& value, void *p) {
  int64_t v = convert_bytes_to_long(value);
  if (v < 0) {
    return false;
  }
  if (p) {
    *((int64_t*)p) = v;
  }
  return true;
}

bool ini_on_update_real(const std::string& value, void *p) {
  if (p) {
    *((double*)p) = zend_strtod(value.c_str(), nullptr);
  }
  return true;
}

bool ini_on_update_stdstring(const std::string& value, void *p) {
  if (p) {
    *((std::string*)p) = value;
  }
  return true;
}

bool ini_on_update_string(const std::string& value, void *p) {
  if (p) {
    *((String*)p) = String(value);
  }
  return true;
}

std::string ini_get_bool(void *p) {
  return (*(bool*) p) ? "1" : "";
}

std::string ini_get_bool_as_int(void* p) {
  return (*(bool*) p) ? "1" : "0";
}

std::string ini_get_long(void *p) {
  return std::to_string(*((int64_t*)p));
}

std::string ini_get_real(void *p) {
  return std::to_string(*((double*)p));
}

std::string ini_get_string(void *p) {
  return ((String*)p)->toCppString();
}

std::string ini_get_stdstring(void *p) {
  return *((std::string*)p);
}

std::string ini_get_static_string_1(void* p) {
  return "1";
}

///////////////////////////////////////////////////////////////////////////////
// callbacks for creating arrays out of ini

void IniSetting::ParserCallback::onSection(const std::string &name, void *arg) {
  // do nothing
}
void IniSetting::ParserCallback::onLabel(const std::string &name, void *arg) {
  // do nothing
}
void IniSetting::ParserCallback::onEntry(
    const std::string &key, const std::string &value, void *arg) {
  Variant *arr = (Variant*)arg;
  arr->set(String(key), String(value));
}
void IniSetting::ParserCallback::onPopEntry(
    const std::string &key, const std::string &value, const std::string &offset,
    void *arg) {
  Variant *arr = (Variant*)arg;
  Variant &hash = arr->lvalAt(String(key));
  if (!hash.isArray()) {
    hash = Array::Create();
  }
  if (!offset.empty()) {
    hash.set(String(offset), String(value));
  } else {
    hash.append(value);
  }
}
void IniSetting::ParserCallback::onConstant(std::string &result,
                                            const std::string &name) {
  if (f_defined(name)) {
    result = f_constant(name).toString().toCppString();
  } else {
    result = name;
  }
}

void IniSetting::ParserCallback::onVar(std::string &result,
                                       const std::string& name) {
  std::string curval;
  if (IniSetting::Get(name, curval)) {
    result = curval;
    return;
  }
  char *value = getenv(name.data());
  if (value) {
    result = std::string(value);
    return;
  }
  result.clear();
}

void IniSetting::ParserCallback::onOp(
    std::string &result, char type, const std::string& op1,
    const std::string& op2) {
  int i_op1 = strtoll(op1.c_str(), nullptr, 10);
  int i_op2 = strtoll(op2.c_str(), nullptr, 10);
  int i_result = 0;
  switch (type) {
    case '|': i_result = i_op1 | i_op2; break;
    case '&': i_result = i_op1 & i_op2; break;
    case '^': i_result = i_op1 ^ i_op2; break;
    case '~': i_result = ~i_op1;        break;
    case '!': i_result = !i_op1;        break;
  }
  result = std::to_string((int64_t)i_result);
}

void IniSetting::SectionParserCallback::onSection(
    const std::string &name, void *arg) {
  CallbackData *data = (CallbackData*)arg;
  data->active_section.unset(); // break ref() from previous section
  data->active_section = Array::Create();
  data->arr.set(String(name), ref(data->active_section));
}
Variant* IniSetting::SectionParserCallback::activeArray(CallbackData* data) {
  if (!data->active_section.isNull()) {
    return &data->active_section;
  } else {
    return &data->arr;
  }
}
void IniSetting::SectionParserCallback::onLabel(const std::string &name,
                                                void *arg) {
  IniSetting::ParserCallback::onLabel(name, activeArray((CallbackData*)arg));
}
void IniSetting::SectionParserCallback::onEntry(
    const std::string &key, const std::string &value, void *arg) {
  IniSetting::ParserCallback::onEntry(key, value,
                                      activeArray((CallbackData*)arg));
}
void IniSetting::SectionParserCallback::onPopEntry(
    const std::string &key, const std::string &value, const std::string &offset,
    void *arg) {
  IniSetting::ParserCallback::onPopEntry(key, value, offset,
                                         activeArray((CallbackData*)arg));
}

void IniSetting::SystemParserCallback::onSection(const std::string &name,
                                                 void *arg) {
  // do nothing
}
void IniSetting::SystemParserCallback::onLabel(const std::string &name,
                                               void *arg) {
  // do nothing
}
void IniSetting::SystemParserCallback::onEntry(
    const std::string &key, const std::string &value, void *arg) {
  assert(!key.empty());
  auto& arr = *(IniSetting::Map*)arg;
  arr[key] = value;
}
void IniSetting::SystemParserCallback::onPopEntry(
    const std::string &key, const std::string &value, const std::string &offset,
    void *arg) {
  assert(!key.empty());
  auto& arr = *(IniSetting::Map*)arg;
  auto* ptr = arr.get_ptr(key);
  if (!ptr || !ptr->isArray()) {
    arr[key] = IniSetting::Map::object;
    ptr = arr.get_ptr(key);
  }
  if (!offset.empty()) {
    (*ptr)[offset] = value;
  } else {
    // Find the highest index
    auto max = 0;
    for (auto &a : ptr->keys()) {
      if (a.isInt() && a > max) {
        max = a.asInt();
      }
    }
    (*ptr)[max] = value;
  }
}
void IniSetting::SystemParserCallback::onConstant(std::string &result,
                                                  const std::string &name) {
  if (f_defined(name, false)) {
    result = f_constant(name).toString().toCppString();
  } else {
    result = name;
  }
}

///////////////////////////////////////////////////////////////////////////////

static Mutex s_mutex;
Variant IniSetting::FromString(const String& ini, const String& filename,
                               bool process_sections, int scanner_mode) {
  Lock lock(s_mutex); // ini parser is not thread-safe
  auto ini_cpp = ini.toCppString();
  auto filename_cpp = filename.toCppString();
  if (process_sections) {
    SectionParserCallback::CallbackData data;
    SectionParserCallback cb;
    data.arr = Array::Create();
    if (zend_parse_ini_string(ini_cpp, filename_cpp, scanner_mode, cb, &data)) {
      return data.arr;
    }
  } else {
    ParserCallback cb;
    Variant ret = Array::Create();
    if (zend_parse_ini_string(ini_cpp, filename_cpp, scanner_mode, cb, &ret)) {
      return ret;
    }
  }

  return false;
}

IniSetting::Map IniSetting::FromStringAsMap(const std::string& ini,
                                            const std::string& filename) {
  Lock lock(s_mutex); // ini parser is not thread-safe
  SystemParserCallback cb;
  Map ret = IniSetting::Map::object;
  zend_parse_ini_string(ini, filename, NormalScanner, cb, &ret);
  return ret;
}

struct IniCallbackData {
  const Extension* extension;
  IniSetting::Mode mode;
  IniSetting::UpdateCallback updateCallback;
  IniSetting::GetCallback getCallback;
  void *p;
};

typedef std::map<std::string, IniCallbackData> CallbackMap;
static IMPLEMENT_THREAD_LOCAL(CallbackMap, s_callbacks);

// This can't be the same as s_callbacks since some classes register callbacks
// before g_context is ready to have the shutdown handler registered
class IniSettingExtension : public Extension {
public:
  IniSettingExtension() : Extension("hhvm.ini", NO_EXTENSION_VERSION_YET) {}
  void requestShutdown() {
    s_callbacks->clear();
  }
} s_ini_extension;

typedef std::map<std::string, std::string> DefaultMap;
static DefaultMap s_global_ini;

void IniSetting::SetGlobalDefault(const char *name, const char *value) {
  assert(name && *name);
  assert(value);
  assert(!Extension::ModulesInitialised());

  s_global_ini[name] = value;
}

void IniSetting::Bind(const Extension* extension, const Mode mode,
                      const char *name, const char *value,
                      UpdateCallback updateCallback, GetCallback getCallback,
                      void *p /* = NULL */) {
  assert(value);

  Bind(extension, mode, name, updateCallback, getCallback, p);

  updateCallback(value, p);
}

void IniSetting::Bind(const Extension* extension, const Mode mode,
                      const char *name,
                      std::string *p) {
  Bind(extension, mode, name, ini_on_update_stdstring, ini_get_stdstring, p);
}
void IniSetting::Bind(const Extension* extension, const Mode mode,
                      const char *name, const char* value,
                      std::string *p) {
  Bind(extension, mode, name, value,
       ini_on_update_stdstring, ini_get_stdstring, p);
}

void IniSetting::Bind(const Extension* extension, const Mode mode,
                      const char *name,
                      String *p) {
  Bind(extension, mode, name, ini_on_update_string, ini_get_string, p);
}
void IniSetting::Bind(const Extension* extension, const Mode mode,
                      const char *name, const char* value,
                      String *p) {
  Bind(extension, mode, name, value, ini_on_update_string, ini_get_string, p);
}

void IniSetting::Bind(const Extension* extension, const Mode mode,
                      const char *name,
                      bool *p) {
  Bind(extension, mode, name, ini_on_update_bool, ini_get_bool, p);
}
void IniSetting::Bind(const Extension* extension, const Mode mode,
                      const char *name, const char *value,
                      bool *p) {
  Bind(extension, mode, name, value, ini_on_update_bool, ini_get_bool, p);
}

void IniSetting::Bind(const Extension* extension, const Mode mode,
                      const char *name,
                      int64_t *p) {
  Bind(extension, mode, name, ini_on_update_long, ini_get_long, p);
}
void IniSetting::Bind(const Extension* extension, const Mode mode,
                      const char *name, const char *value,
                      int64_t *p) {
  Bind(extension, mode, name, value, ini_on_update_long, ini_get_long, p);
}

void IniSetting::Bind(const Extension* extension, const Mode mode,
                      const char *name,
                      UpdateCallback updateCallback, GetCallback getCallback,
                      void *p /* = NULL */) {
  assert(name && *name);

  auto &data = (*s_callbacks)[name];
  data.extension = extension;
  data.mode = mode;
  data.updateCallback = updateCallback;
  data.getCallback = getCallback;
  data.p = p;
}

void IniSetting::Unbind(const char *name) {
  assert(name && *name);
  s_callbacks->erase(name);
}

bool IniSetting::Get(const std::string& name, std::string &value) {
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

bool IniSetting::Get(const String& name, String &value) {
  std::string b;
  auto ret = Get(name.toCppString(), b);
  value = b;
  return ret;
}

static bool ini_set(const String& name, const String& value,
                    IniSetting::Mode mode) {
  CallbackMap::iterator iter = s_callbacks->find(name.data());
  if (iter != s_callbacks->end()) {
    if ((iter->second.mode & mode) && iter->second.updateCallback) {
      return iter->second.updateCallback(value.toCppString(), iter->second.p);
    }
  }
  return false;
}

bool IniSetting::Set(const String& name, const String& value) {
  return ini_set(name, value, static_cast<Mode>(
    PHP_INI_ONLY | PHP_INI_SYSTEM | PHP_INI_PERDIR | PHP_INI_USER | PHP_INI_ALL
  ));
}

bool IniSetting::SetUser(const String& name, const String& value) {
  return ini_set(name, value, static_cast<Mode>(
    PHP_INI_USER | PHP_INI_ALL
  ));
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
      auto value = iter.second.getCallback(iter.second.p);
      item.add(s_global_value, value);
      item.add(s_local_value, value);
      if (iter.second.mode == PHP_INI_ALL) {
        item.add(
          s_access,
          Variant(PHP_INI_USER | PHP_INI_SYSTEM | PHP_INI_PERDIR)
        );
      } else if (iter.second.mode == PHP_INI_ONLY) {
        item.add(s_access, Variant(PHP_INI_SYSTEM));
      } else {
        item.add(s_access, Variant(iter.second.mode));
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
