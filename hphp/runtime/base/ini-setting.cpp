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

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/hphp-system.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/zend-strtod.h"

#include "hphp/runtime/base/ini-parser/zend-ini.h"

#include "hphp/runtime/ext/ext_misc.h"
#include "hphp/runtime/ext/extension.h"

#include "hphp/util/lock.h"

#define __STDC_LIMIT_MACROS
#include <cstdint>
#include <boost/range/join.hpp>
#include <map>

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
  if (value.size() == 0) {
    return 0;
  }
  int64_t newInt = strtoll(value.data(), nullptr, 10);
  char lastChar = value.data()[value.size() - 1];
  if (lastChar == 'K' || lastChar == 'k') {
    newInt <<= 10;
  } else if (lastChar == 'M' || lastChar == 'm') {
    newInt <<= 20;
  } else if (lastChar == 'G' || lastChar == 'g') {
    newInt <<= 30;
  }
  return newInt;
}

static std::string dynamic_to_std_string(const folly::dynamic& v) {
  switch (v.type()) {
    case folly::dynamic::Type::NULLT:
    case folly::dynamic::Type::ARRAY:
    case folly::dynamic::Type::OBJECT:
      return "";
    case folly::dynamic::Type::BOOL:
      return std::to_string(v.asBool());
    case folly::dynamic::Type::DOUBLE:
      return std::to_string(v.asDouble());
    case folly::dynamic::Type::INT64:
      return std::to_string(v.asInt());
    case folly::dynamic::Type::STRING:
      return v.data();
  }
  not_reached();
}

/**
 * I was going to make this a constructor for Variant, but both folly::dynamic
 * and Variant have so many overrides that everything becomes ambiguous.
 **/
static Variant dynamic_to_variant(const folly::dynamic& v) {
  switch (v.type()) {
    case folly::dynamic::Type::NULLT:
      return init_null_variant;
    case folly::dynamic::Type::BOOL:
      return v.asBool();
    case folly::dynamic::Type::DOUBLE:
      return v.asDouble();
    case folly::dynamic::Type::INT64:
      return v.asInt();
    case folly::dynamic::Type::STRING:
      return v.data();
    case folly::dynamic::Type::ARRAY:
    case folly::dynamic::Type::OBJECT:
      ArrayInit ret(v.size());
      for (auto& item : v.items()) {
        ret.add(dynamic_to_variant(item.first),
                dynamic_to_variant(item.second));
      }
      return ret.toArray();
  }
  not_reached();
}

static folly::dynamic variant_to_dynamic(const Variant& v) {
  switch (v.getType()) {
    case KindOfUninit:
    case KindOfNull:
    default:
      return nullptr;
    case KindOfBoolean:
      return v.toBoolean();
    case KindOfDouble:
      return v.toDouble();
    case KindOfInt64:
      return v.toInt64();
    case KindOfString:
    case KindOfStaticString:
      return v.toString().data();
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
      folly::dynamic ret = folly::dynamic::object;
      for (ArrayIter iter(v.toArray()); iter; ++iter) {
        ret.insert(variant_to_dynamic(iter.first()),
                   variant_to_dynamic(iter.second()));
      }
      return ret;
  }
  not_reached();
}

#define INI_ASSERT_STR(v) \
  if (value.isArray() || value.isObject()) { \
    return false; \
  } \
  auto str = dynamic_to_std_string(v);

#define INI_ASSERT_ARR(v) \
  if (!value.isArray() && !value.isObject()) { \
    return false; \
  }

bool ini_on_update(const folly::dynamic& value, bool& p) {
  INI_ASSERT_STR(value);
  if ((str.size() == 0) ||
      (str.size() == 1 && strcasecmp("0", str.data()) == 0) ||
      (str.size() == 2 && strcasecmp("no", str.data()) == 0) ||
      (str.size() == 3 && strcasecmp("off", str.data()) == 0) ||
      (str.size() == 5 && strcasecmp("false", str.data()) == 0)) {
    p = false;
  } else {
    p = true;
  }
  return true;
}

bool ini_on_update(const folly::dynamic& value, double& p) {
  INI_ASSERT_STR(value);
  p = zend_strtod(str.data(), nullptr);
  return true;
}

bool ini_on_update(const folly::dynamic& value, int16_t& p) {
  INI_ASSERT_STR(value);
  auto n = convert_bytes_to_long(str);
  auto maxValue = 0x7FFFL;
  if (n > maxValue || n < (- maxValue - 1)) {
    return false;
  }
  p = n;
  return true;
}

bool ini_on_update(const folly::dynamic& value, int32_t& p) {
  INI_ASSERT_STR(value);
  auto n = convert_bytes_to_long(str);
  auto maxValue = 0x7FFFFFFFL;
  if (n > maxValue || n < (- maxValue - 1)) {
    return false;
  }
  p = n;
  return true;
}

bool ini_on_update(const folly::dynamic& value, int64_t& p) {
  INI_ASSERT_STR(value);
  p = convert_bytes_to_long(str);
  return true;
}

bool ini_on_update(const folly::dynamic& value, uint16_t& p) {
  INI_ASSERT_STR(value);
  auto n = convert_bytes_to_long(str);
  auto mask = ~0xFFFFUL;
  if (((uint64_t)n & mask)) {
    return false;
  }
  p = n;
  return true;
}

bool ini_on_update(const folly::dynamic& value, uint32_t& p) {
  INI_ASSERT_STR(value);
  auto n = convert_bytes_to_long(str);
  auto mask = ~0x7FFFFFFFUL;
  if (((uint64_t)n & mask)) {
    return false;
  }
  p = n;
  return true;
}

bool ini_on_update(const folly::dynamic& value, uint64_t& p) {
  INI_ASSERT_STR(value);
  p = convert_bytes_to_long(str);
  return true;
}

bool ini_on_update(const folly::dynamic& value, std::string& p) {
  INI_ASSERT_STR(value);
  p = str;
  return true;
}

bool ini_on_update(const folly::dynamic& value, String& p) {
  INI_ASSERT_STR(value);
  p = str.data();
  return true;
}

bool ini_on_update(const folly::dynamic& value, Array& p) {
  INI_ASSERT_ARR(value);
  p = dynamic_to_variant(value).toArray();
  return true;
}

bool ini_on_update(const folly::dynamic& value, std::set<std::string>& p) {
  INI_ASSERT_ARR(value);
  for (auto& v : value.values()) {
    p.insert(v.data());
  }
  return true;
}

folly::dynamic ini_get(bool& p) {
  return p ? "1" : "";
}

folly::dynamic ini_get(double& p) {
  return p;
}

folly::dynamic ini_get(int16_t& p) {
  return p;
}

folly::dynamic ini_get(int32_t& p) {
  return p;
}

folly::dynamic ini_get(int64_t& p) {
  return p;
}

folly::dynamic ini_get(uint16_t& p) {
  return p;
}

folly::dynamic ini_get(uint32_t& p) {
  return p;
}

folly::dynamic ini_get(uint64_t& p) {
  return p;
}

folly::dynamic ini_get(std::string& p) {
  return p.data();
}

folly::dynamic ini_get(String& p) {
  return p.data();
}

folly::dynamic ini_get(Array& p) {
  folly::dynamic ret = folly::dynamic::object;
  for (ArrayIter iter(p); iter; ++iter) {
    ret.insert(variant_to_dynamic(iter.first()),
               variant_to_dynamic(iter.second()));
  }
  return ret;
}

folly::dynamic ini_get(std::set<std::string>& p) {
  folly::dynamic ret = folly::dynamic::object;
  for (auto& s : p) {
    ret.push_back(s);
  }
  return ret;
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
  forceToArray(*arr).set(String(key), String(value));
}

void IniSetting::ParserCallback::onPopEntry(
    const std::string &key,
    const std::string &value,
    const std::string &offset,
    void *arg) {
  Variant *arr = (Variant*)arg;
  forceToArray(*arr);
  auto& hash = arr->toArrRef().lvalAt(String(key));
  forceToArray(hash);
  if (!offset.empty()) {
    makeArray(hash, offset, value);
  } else {
    hash.toArrRef().append(value);
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
    Variant newval;
    if (last) {
      newval = Variant(value);
    } else {
      if (val.toArrRef().exists(index)) {
        newval = val.toArrRef().rvalAt(index);
      } else {
        newval = Variant(Array::Create());
      }
    }
    val.toArrRef().setRef(index, newval);
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
  auto const data = (CallbackData*)arg;
  data->active_section.unset(); // break ref() from previous section
  data->active_section = Array::Create();
  data->arr.toArrRef().setRef(String(name), data->active_section);
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

void IniSetting::SystemParserCallback::onPopEntry(const std::string& key,
                                                  const std::string& value,
                                                  const std::string& offset,
                                                  void* arg) {
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
  if (MemoryManager::TlsWrapper::isNull()) {
    // We can't load constants before the memory manger is up, so lets just
    // pretend they are strings I guess
    result = name;
    return;
  }

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
    CallbackData data;
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
  std::function<bool(const folly::dynamic& value)> updateCallback;
  std::function<folly::dynamic()> getCallback;
};

typedef std::map<std::string, IniCallbackData> CallbackMap;
// Things that the user can change go here
static IMPLEMENT_THREAD_LOCAL(CallbackMap, s_user_callbacks);
// Things that are only settable at startup go here
static CallbackMap s_system_ini_callbacks;

typedef std::map<std::string, folly::dynamic> DefaultMap;
static IMPLEMENT_THREAD_LOCAL(DefaultMap, s_savedDefaults);

class IniSettingExtension : public Extension {
public:
  IniSettingExtension() : Extension("hhvm.ini", NO_EXTENSION_VERSION_YET) {}

  void requestShutdown() {
    // Put all the defaults back to the way they were before any ini_set()
    for (auto &item : *s_savedDefaults) {
      IniSetting::Set(item.first, item.second, IniSetting::FollyDynamic());
    }
    s_savedDefaults->clear();
  }

} s_ini_extension;

void IniSetting::Bind(const Extension* extension, const Mode mode,
                      const char *name,
                      std::function<bool(const folly::dynamic& value)>
                        updateCallback,
                      std::function<folly::dynamic()> getCallback) {
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

static IniCallbackData* get_callback(const std::string& name) {
  CallbackMap::iterator iter = s_system_ini_callbacks.find(name.data());
  if (iter == s_system_ini_callbacks.end()) {
    iter = s_user_callbacks->find(name.data());
    if (iter == s_user_callbacks->end()) {
      return nullptr;
    }
  }
  return &iter->second;
}

bool IniSetting::Get(const std::string& name, folly::dynamic& value) {
  auto cb = get_callback(name);
  if (!cb) {
    return false;
  }
  value = cb->getCallback();
  return true;
}

bool IniSetting::Get(const std::string& name, std::string &value) {
  folly::dynamic b = nullptr;
  auto ret = Get(name, b);
  value = dynamic_to_std_string(b);
  return ret && !value.empty();
}

bool IniSetting::Get(const String& name, String& value) {
  Variant b;
  auto ret = Get(name, b);
  value = b.toString();
  return ret;
}

bool IniSetting::Get(const String& name, Variant& value) {
  folly::dynamic b = nullptr;
  auto ret = Get(name.toCppString(), b);
  value = dynamic_to_variant(b);
  return ret;
}

std::string IniSetting::Get(const std::string& name) {
  std::string ret;
  Get(name, ret);
  return ret;
}

static bool ini_set(const std::string& name, const folly::dynamic& value,
                    IniSetting::Mode mode) {
  auto cb = get_callback(name);
  if (!cb || !(cb->mode & mode)) {
    return false;
  }
  return cb->updateCallback(value);
}

bool IniSetting::Set(const std::string& name, const folly::dynamic& value,
                     FollyDynamic) {
  return ini_set(name, value, static_cast<Mode>(
    PHP_INI_ONLY | PHP_INI_SYSTEM | PHP_INI_PERDIR | PHP_INI_USER | PHP_INI_ALL
  ));
}

bool IniSetting::Set(const String& name, const Variant& value) {
  return Set(name.toCppString(), variant_to_dynamic(value), FollyDynamic());
}

bool IniSetting::SetUser(const std::string& name, const folly::dynamic& value,
                         FollyDynamic) {
  auto it = s_savedDefaults->find(name);
  if (it == s_savedDefaults->end()) {
    folly::dynamic def = nullptr;
    auto success = Get(name, def);
    if (success) {
      s_savedDefaults->insert(make_pair(name, def));
    }
  }
  return ini_set(name, value, static_cast<Mode>(
    PHP_INI_USER | PHP_INI_ALL
  ));
}

bool IniSetting::SetUser(const String& name, const Variant& value) {
  return SetUser(name.toCppString(), variant_to_dynamic(value), FollyDynamic());
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

    auto value = dynamic_to_variant(iter.second.getCallback());
    // Cast all non-arrays to strings since that is what everything used ot be
    if (!value.isArray()) {
      value = value.toString();
    }
    if (details) {
      Array item = Array::Create();
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
      r.add(String(iter.first), value);
    }
  }
  return r;
}

///////////////////////////////////////////////////////////////////////////////
}
