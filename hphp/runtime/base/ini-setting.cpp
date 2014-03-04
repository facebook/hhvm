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
#include <boost/range/join.hpp>
#include <map>

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

bool IniSetting::s_pretendExtensionsHaveNotBeenLoaded = false;

const StaticString
  s_global_value("global_value"),
  s_local_value("local_value"),
  s_access("access"),
  s_core("core");

int64_t convert_bytes_to_long(const std::string& value) {
  if (value.empty()) {
    return 0;
  }

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

bool ini_on_update(const std::string& value, bool& p) {
  if ((value.size() == 0) ||
      (value.size() == 1 && value == "0") ||
      (value.size() == 2 && strcasecmp("no", value.data()) == 0) ||
      (value.size() == 3 && strcasecmp("off", value.data()) == 0) ||
      (value.size() == 5 && strcasecmp("false", value.data()) == 0)) {
    p = false;
  } else {
    p = true;
  }
  return true;
}

bool ini_on_update(const std::string& value, double& p) {
  p = zend_strtod(value.c_str(), nullptr);
  return true;
}

bool ini_on_update(const std::string& value, int16_t& p) {
  auto n = convert_bytes_to_long(value);
  auto maxValue = 0x7FFFL;
  if (n > maxValue || n < (- maxValue - 1)) {
    return false;
  }
  p = n;
  return true;
}

bool ini_on_update(const std::string& value, int32_t& p) {
  auto n = convert_bytes_to_long(value);
  auto maxValue = 0x7FFFFFFFL;
  if (n > maxValue || n < (- maxValue - 1)) {
    return false;
  }
  p = n;
  return true;
}

bool ini_on_update(const std::string& value, int64_t& p) {
  p = convert_bytes_to_long(value);
  return true;
}

bool ini_on_update(const std::string& value, uint16_t& p) {
  auto n = convert_bytes_to_long(value);
  auto mask = ~0xFFFFUL;
  if (((uint64_t)n & mask)) {
    return false;
  }
  p = n;
  return true;
}

bool ini_on_update(const std::string& value, uint32_t& p) {
  auto n = convert_bytes_to_long(value);
  auto mask = ~0x7FFFFFFFUL;
  if (((uint64_t)n & mask)) {
    return false;
  }
  p = n;
  return true;
}

bool ini_on_update(const std::string& value, uint64_t& p) {
  p = convert_bytes_to_long(value);
  return true;
}

bool ini_on_update(const std::string& value, std::string& p) {
  p = value;
  return true;
}

bool ini_on_update(const std::string& value, String& p) {
  p = String(value);
  return true;
}

std::string ini_get(bool& p) {
  return p ? "1" : "";
}

std::string ini_get(double& p) {
  return std::to_string(p);
}

std::string ini_get(int16_t& p) {
  return std::to_string(p);
}

std::string ini_get(int32_t& p) {
  return std::to_string(p);
}

std::string ini_get(int64_t& p) {
  return std::to_string(p);
}

std::string ini_get(uint16_t& p) {
  return std::to_string(p);
}

std::string ini_get(uint32_t& p) {
  return std::to_string(p);
}

std::string ini_get(uint64_t& p) {
  return std::to_string(p);
}

std::string ini_get(std::string& p) {
  return p;
}

std::string ini_get(String& p) {
  return p->toCppString();
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
    makeArray(hash, offset, value);
  } else {
    hash.append(value);
  }
}
void IniSetting::ParserCallback::makeArray(Variant &hash,
                                           const std::string &offset,
                                           const std::string &value) {
  assert(!offset.empty());
  Variant val = strongBind(hash);
  auto start = offset.c_str();
  auto p = start;
  bool last = false;
  do {
    String index(p);
    last = p + index.size() >= start + offset.size();
    auto def = Variant(Array::Create());
    Variant newval = last ? Variant(value) : val.lvalRef(index, def);
    val.setRef(index, newval);
    if (!last) {
      val = strongBind(newval);
      p += index.size() + 1;
    }
  } while (!last);
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
  if (!ptr || !ptr->isObject()) {
    arr[key] = IniSetting::Map::object;
    ptr = arr.get_ptr(key);
  }
  if (!offset.empty()) {
    makeArray(*ptr, offset, value);
  } else {
    // Find the highest index
    auto max = 0;
    for (auto &a : ptr->keys()) {
      if (a.isInt() && a >= max) {
        max = a.asInt() + 1;
      }
    }
    (*ptr)[std::to_string(max)] = value;
  }
}
void IniSetting::SystemParserCallback::makeArray(Map &hash,
                                                 const std::string &offset,
                                                 const std::string &value) {
  assert(!offset.empty());
  Map& val = hash;
  auto start = offset.c_str();
  auto p = start;
  bool last = false;
  do {
    std::string index(p);
    last = p + index.size() >= start + offset.size();
    Map newval = last ? Map(value) : val.getDefault(index, Map::object());
    val[index] = newval;
    if (!last) {
      val = newval;
      p += index.size() + 1;
    }
  } while (!last);
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
  std::function<bool(const std::string& value)> updateCallback;
  std::function<std::string()> getCallback;
};

typedef std::map<std::string, IniCallbackData> CallbackMap;
// Things that the user can change go here
static IMPLEMENT_THREAD_LOCAL(CallbackMap, s_user_callbacks);
// Things that are only settable at startup go here
static CallbackMap s_system_ini_callbacks;

typedef std::map<std::string, std::string> DefaultMap;
static IMPLEMENT_THREAD_LOCAL(DefaultMap, s_savedDefaults);

class IniSettingExtension : public Extension {
public:
  IniSettingExtension() : Extension("hhvm.ini", NO_EXTENSION_VERSION_YET) {}

  void requestShutdown() {
    // Put all the defaults back to the way they were before any ini_set()
    for (auto &item : *s_savedDefaults) {
      IniSetting::Set(item.first, item.second);
    }
    s_savedDefaults->clear();
  }

} s_ini_extension;

void IniSetting::Bind(const Extension* extension, const Mode mode,
                      const char *name,
                      std::function<bool(const std::string& value)>
                        updateCallback,
                      std::function<std::string()> getCallback) {
  assert(name && *name);

  bool is_thread_local = (mode == PHP_INI_USER || mode == PHP_INI_ALL);
  // For now, we require the extensions to use their own thread local memory for
  // user-changeable settings. This means you need to use the default field to
  // Bind and can't statically initialize them. We could conceivably let you
  // use static memory and have our own thread local here that users can change
  // and then reset it back to the default, but we haven't built that yet.
  auto &data = is_thread_local ? (*s_user_callbacks)[name]
                               : s_system_ini_callbacks[name];
  // I would love if I could verify p is thread local or not instead of
  // this dumb hack
  assert(is_thread_local || !Extension::ModulesInitialised() ||
         s_pretendExtensionsHaveNotBeenLoaded);

  data.extension = extension;
  data.mode = mode;
  data.updateCallback = updateCallback;
  data.getCallback = getCallback;
}

void IniSetting::Unbind(const char *name) {
  assert(name && *name);
  s_user_callbacks->erase(name);
}

bool IniSetting::Get(const std::string& name, std::string &value) {
  CallbackMap::iterator iter = s_system_ini_callbacks.find(name.data());
  if (iter == s_system_ini_callbacks.end()) {
    iter = s_user_callbacks->find(name.data());
    if (iter == s_user_callbacks->end()) {
      return false;
    }
  }

  value = iter->second.getCallback();
  return true;
}

bool IniSetting::Get(const String& name, String &value) {
  std::string b;
  auto ret = Get(name.toCppString(), b);
  value = b;
  return ret;
}

std::string IniSetting::Get(const std::string& name) {
  std::string ret;
  Get(name, ret);
  return ret;
}

static bool ini_set(const String& name, const Variant& value,
                    IniSetting::Mode mode) {
  CallbackMap::iterator iter = s_user_callbacks->find(name.data());
  if (iter != s_user_callbacks->end()) {
    if ((iter->second.mode & mode) && iter->second.updateCallback) {
      return iter->second.updateCallback(value.toString().toCppString());
    }
  }
  return false;
}

bool IniSetting::Set(const String& name, const Variant& value) {
  return ini_set(name, value, static_cast<Mode>(
    PHP_INI_ONLY | PHP_INI_SYSTEM | PHP_INI_PERDIR | PHP_INI_USER | PHP_INI_ALL
  ));
}

bool IniSetting::SetUser(const String& nameString, const Variant& value) {
  auto name = nameString.toCppString();
  auto it = s_savedDefaults->find(name);
  if (it == s_savedDefaults->end()) {
    Get(name, (*s_savedDefaults)[name]);
  }
  return ini_set(nameString, value, static_cast<Mode>(
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

  for (auto& iter: boost::join(s_system_ini_callbacks, *s_user_callbacks)) {
    if (ext && ext != iter.second.extension) {
      continue;
    }

    if (details) {
      Array item = Array::Create();
      auto value = iter.second.getCallback();
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
      r.add(String(iter.first), iter.second.getCallback());
    }
  }
  return r;
}

///////////////////////////////////////////////////////////////////////////////
}
