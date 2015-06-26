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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/zend-strtod.h"

#include "hphp/runtime/base/ini-parser/zend-ini.h"

#include "hphp/runtime/ext/std/ext_std_misc.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/extension-registry.h"

#include "hphp/util/lock.h"

#include <glob.h>

#define __STDC_LIMIT_MACROS
#include <cstdint>
#include <boost/range/join.hpp>
#include <map>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const Extension* IniSetting::CORE = (Extension*)(-1);

bool IniSetting::s_pretendExtensionsHaveNotBeenLoaded = false;

bool IniSetting::s_config_is_a_constant = false;
std::set<std::string> IniSetting::config_names_that_use_constants;

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
      return convDblToStrWithPhpFormat(v.asDouble());
    case folly::dynamic::Type::INT64:
      return std::to_string(v.asInt());
    case folly::dynamic::Type::STRING:
      return v.data();
  }
  not_reached();
}

static Variant dynamic_to_variant(const folly::dynamic& v) {
  switch (v.type()) {
    case folly::dynamic::Type::NULLT:
      return init_null();
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
      ArrayInit arr_init(v.size(), ArrayInit::Mixed{});
      for (auto& item : v.items()) {
        arr_init.add(dynamic_to_variant(item.first),
                dynamic_to_variant(item.second));
      }
      Array ret = arr_init.toArray();
      // Sort the array since folly::dynamic has a tendency to iterate from
      // back to front. This way a var_dump of the array, for example, looks
      // ordered.
      ret.sort(Array::SortNaturalAscending, true, false);
      return ret;
  }
  not_reached();
}

static folly::dynamic variant_to_dynamic(const Variant& v) {
  switch (v.getType()) {
    case KindOfUninit:
    case KindOfNull:
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
    case KindOfResource: {
      folly::dynamic ret = folly::dynamic::object;
      for (ArrayIter iter(v.toArray()); iter; ++iter) {
        ret.insert(variant_to_dynamic(iter.first()),
                   variant_to_dynamic(iter.second()));
      }
      return ret;
    }
    case KindOfRef:
    case KindOfClass:
      break;
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

bool ini_on_update(const folly::dynamic& value, char& p) {
  INI_ASSERT_STR(value);
  auto n = convert_bytes_to_long(str);
  auto maxValue = 0x7FL;
  if (n > maxValue || n < (- maxValue - 1)) {
    return false;
  }
  p = n;
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

bool ini_on_update(const folly::dynamic& value, unsigned char& p) {
  INI_ASSERT_STR(value);
  auto n = convert_bytes_to_long(str);
  auto mask = ~0xFFUL;
  if (((uint64_t)n & mask)) {
    return false;
  }
  p = n;
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

bool ini_on_update(const folly::dynamic& value,
                   std::set<std::string, stdltistr>& p) {
  INI_ASSERT_ARR(value);
  for (auto& v : value.values()) {
    p.insert(v.data());
  }
  return true;
}

bool ini_on_update(const folly::dynamic& value,
                   boost::container::flat_set<std::string>& p) {
  INI_ASSERT_ARR(value);
  for (auto& v : value.values()) {
    p.insert(v.data());
  }
  return true;
}

bool ini_on_update(const folly::dynamic& value, std::vector<std::string>& p) {
  INI_ASSERT_ARR(value);
  for (auto& v : value.values()) {
    p.push_back(v.data());
  }
  return true;
}

bool ini_on_update(const folly::dynamic& value,
                   std::map<std::string, std::string>& p) {
  INI_ASSERT_ARR(value);
  for (auto& pair : value.items()) {
    p[pair.first.data()] = pair.second.data();
  }
  return true;
}

bool ini_on_update(const folly::dynamic& value,
                   std::map<std::string, std::string, stdltistr>& p) {
  INI_ASSERT_ARR(value);
  for (auto& pair : value.items()) {
    p[pair.first.data()] = pair.second.data();
  }
  return true;
}

bool ini_on_update(const folly::dynamic& value,
                   hphp_string_imap<std::string>& p) {
  INI_ASSERT_ARR(value);
  for (auto& pair : value.items()) {
    p[pair.first.data()] = pair.second.data();
  }
  return true;
}

folly::dynamic ini_get(bool& p) {
  return p ? "1" : "";
}

folly::dynamic ini_get(double& p) {
  return p;
}

folly::dynamic ini_get(char& p) {
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

folly::dynamic ini_get(unsigned char& p) {
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

folly::dynamic ini_get(std::map<std::string, std::string>& p) {
  folly::dynamic ret = folly::dynamic::object;
  for (auto& pair : p) {
    ret.insert(pair.first, pair.second);
  }
  return ret;
}

folly::dynamic ini_get(std::map<std::string, std::string, stdltistr>& p) {
  folly::dynamic ret = folly::dynamic::object;
  for (auto& pair : p) {
    ret.insert(pair.first, pair.second);
  }
  return ret;
}

folly::dynamic ini_get(hphp_string_imap<std::string>& p) {
  folly::dynamic ret = folly::dynamic::object;
  for (auto& pair : p) {
    ret.insert(pair.first, pair.second);
  }
  return ret;
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
  auto idx = 0;
  for (auto& s : p) {
    ret.insert(idx++, s);
  }
  return ret;
}

folly::dynamic ini_get(std::set<std::string, stdltistr>& p) {
  folly::dynamic ret = folly::dynamic::object;
  auto idx = 0;
  for (auto& s : p) {
    ret.insert(idx++, s);
  }
  return ret;
}

folly::dynamic ini_get(boost::container::flat_set<std::string>& p) {
  folly::dynamic ret = folly::dynamic::object;
  auto idx = 0;
  for (auto& s : p) {
    ret.insert(idx++, s);
  }
  return ret;
}

folly::dynamic ini_get(std::vector<std::string>& p) {
  folly::dynamic ret = folly::dynamic::object;
  auto idx = 0;
  for (auto& s : p) {
    ret.insert(idx++, s);
  }
  return ret;
}

const folly::dynamic* ini_iterate(const folly::dynamic &ini,
                                  const std::string &name) {
  // This should never happen, but handle it anyway.
  if (ini == nullptr) {
    return nullptr;
  }

  // If for some reason we are passed a string (i.e., a leaf value),
  // just return it back
  if (ini.isString()) {
    return &ini;
  }

  // If we just passed in a name that already has a value like:
  //   hhvm.server.apc.ttl_limit
  //   max_execution_time
  // then we just return the value now.
  // i.e., a value that didn't look like
  //   hhvm.a.b[c][d], where name = hhvm.a.b.c.d
  //   c[d] (where ini is already hhvm.a.b), where name = c.d
  auto* value = ini.get_ptr(name);
  if (value) {
    return value;
  }

  // Otherwise, we split on the dots (if any) to see if we can get a real value
  std::vector<std::string> dot_parts;
  folly::split('.', name, dot_parts);

  int dot_loc = 0;
  int dot_parts_size = dot_parts.size();
  std::string part = dot_parts[0];
  // If this is null, then all the loops below will be skipped and
  // we will return it as null.
  value = ini.get_ptr(part);
  // Loop through the dot parts, getting a pointer to each
  // We may need to concatenate dots to be able to get a real value
  // e.g., if someone passed in hhvm.a.b.c.d, which in ini was equal
  // to hhvm.a.b[c][d], then we would start with hhvm and get null,
  // then hhvm.a and get null, then hhvm.a.b and actually get an object
  // to point to.
  while (!value && dot_loc < dot_parts_size - 1) {
    dot_loc++;
    part = part + "." + dot_parts[dot_loc];
    value = ini.get_ptr(part);
  }
  // Get to the last dot part and get its value, if it exists
  for (int i = dot_loc + 1; i < dot_parts_size; i++) {
    if (value) {
      part = dot_parts[i];
      value = value->get_ptr(part);
    } else { // If we reach a bad point, just return null
      return nullptr;
    }
  }
  return value;
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

void IniSetting::ParserCallback::makeArray(Variant& hash,
                                           const std::string& offset,
                                           const std::string& value) {
  assert(!offset.empty());
  Variant val(Variant::StrongBind{}, hash);
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
      val.assignRef(newval);
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
  String value = g_context->getenv(name);
  if (!value.isNull()) {
    result = value.toCppString();
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
  // onConstant will always be called before onEntry, so we can check
  // here
  if (IniSetting::s_config_is_a_constant) {
    IniSetting::config_names_that_use_constants.insert(key);
    IniSetting::s_config_is_a_constant = false;
  }
  auto& arr = *(IniSetting::Map*)arg;
  arr[key] = value;
}

void IniSetting::SystemParserCallback::onPopEntry(const std::string& key,
                                                  const std::string& value,
                                                  const std::string& offset,
                                                  void* arg) {
  assert(!key.empty());
  if (IniSetting::s_config_is_a_constant) {
    IniSetting::config_names_that_use_constants.insert(key);
    IniSetting::s_config_is_a_constant = false;
  }
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
      try {
        if (a.asInt() >= max) {
          max = a.asInt() + 1;
        }
      } catch (std::range_error const& e) { /* not an int */ }
    }
    (*ptr)[std::to_string(max)] = value;
  }
}

void IniSetting::SystemParserCallback::makeArray(Map &hash,
                                                 const std::string &offset,
                                                 const std::string &value) {
  assert(!offset.empty());
  Map* val = &hash;
  auto start = offset.c_str();
  auto p = start;
  bool last = false;
  do {
    std::string index(p);
    last = p + index.size() >= start + offset.size();

    Map newval = last ? Map(value) : val->getDefault(index, Map::object());
    val = &(*val)[index];
    *val = newval;

    if (!last) {
      p += index.size() + 1;
    }
  } while (!last);
}
void IniSetting::SystemParserCallback::onConstant(std::string &result,
                                                  const std::string &name) {
  IniSetting::s_config_is_a_constant = true;
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
  // We are parsing something new, so reset this flag
  s_config_is_a_constant = false;
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
  // We are parsing something new, so reset this flag
  s_config_is_a_constant = false;
  SystemParserCallback cb;
  Map ret = IniSetting::Map::object;
  zend_parse_ini_string(ini, filename, NormalScanner, cb, &ret);
  return ret;
}

class IniCallbackData {
public:
  IniCallbackData() {
    extension = nullptr;
    mode = IniSetting::PHP_INI_NONE;
    iniData = nullptr;
    updateCallback = nullptr;
    getCallback = nullptr;
  }
  virtual ~IniCallbackData() {
    delete iniData;
    iniData = nullptr;
  }
public:
  const Extension* extension;
  IniSetting::Mode mode;
  UserIniData *iniData;
  std::function<bool(const folly::dynamic& value)> updateCallback;
  std::function<folly::dynamic()> getCallback;
};

typedef std::map<std::string, IniCallbackData> CallbackMap;

//
// These are for settings/callbacks only settable at startup.
//
// Empirically and surprisingly (20Jan2015):
//   * server mode: the contents of system map are     destructed on SIGTERM
//   * CLI    mode: the contents of system map are NOT destructed on SIGTERM
//
static CallbackMap s_system_ini_callbacks;

//
// These are for settings/callbacks that the script
// can change during the request.
//
// Empirically and surprisingly (20Jan2015), when there are N threads:
//   * server mode: the contents of user map are     destructed N-1 times
//   * CLI    mode: the contents of user map are NOT destructed on SIGTERM
//
static IMPLEMENT_THREAD_LOCAL(CallbackMap, s_user_callbacks);

typedef std::map<std::string, folly::dynamic> SettingMap;
// Set by a .ini file at the start
static SettingMap s_system_settings;
// Changed during the course of the request
static IMPLEMENT_THREAD_LOCAL(SettingMap, s_saved_defaults);

class IniSettingExtension final : public Extension {
public:
  IniSettingExtension() : Extension("hhvm.ini", NO_EXTENSION_VERSION_YET) {}

  void requestShutdown() override {
    // Put all the defaults back to the way they were before any ini_set()
    for (auto &item : *s_saved_defaults) {
      IniSetting::SetUser(item.first, item.second, IniSetting::FollyDynamic());
    }
    s_saved_defaults->clear();
  }

} s_ini_extension;

void IniSetting::Bind(
  const Extension* extension,
  const Mode mode,
  const std::string& name,
  std::function<bool(const folly::dynamic&)> updateCallback,
  std::function<folly::dynamic()> getCallback,
  std::function<class UserIniData *(void)> userDataCallback
) {
  assert(!name.empty());

  /*
   * WATCH OUT: unlike php5, a Mode is not necessarily a bit mask.
   * PHP_INI_ALL is NOT encoded as the union:
   *   PHP_INI_USER|PHP_INI_PERDIR|PHP_INI_SYSTEM
   *
   * Note that Mode value PHP_INI_SET_USER and PHP_INI_SET_EVERY are bit
   * sets; "SET" in this use means "bitset", and not "assignment".
   */
  bool is_thread_local;
  if (RuntimeOption::EnableZendIniCompat) {
    is_thread_local = (
    (mode == PHP_INI_USER) ||
    (mode == PHP_INI_PERDIR) ||
    (mode == PHP_INI_ALL) ||  /* See note above */
    (mode &  PHP_INI_USER) ||
    (mode &  PHP_INI_PERDIR) ||
    (mode &  PHP_INI_ALL)
    );
  } else {
    is_thread_local = (mode == PHP_INI_USER || mode == PHP_INI_ALL);
    assert(is_thread_local || !ExtensionRegistry::modulesInitialised() ||
           s_pretendExtensionsHaveNotBeenLoaded);
  }
  //
  // When the debugger is loading its configuration, there will be some
  // cases where Extension::ModulesInitialised(), but the name appears
  // in neither s_user_callbacks nor s_system_ini_callbacks. The bottom
  // line is that we can't really use ModulesInitialised() to help steer
  // the choices here.
  //

  bool use_user = is_thread_local;
  if (RuntimeOption::EnableZendIniCompat && !use_user) {
    //
    // If it is already in the user callbacks, continue to use it from
    // there. We don't expect it to be already there, but it has been
    // observed during development.
    //
    bool in_user_callbacks =
      (s_user_callbacks->find(name) != s_user_callbacks->end());
    assert (!in_user_callbacks);  // See note above
    use_user = in_user_callbacks;
  }

  //
  // For now, we require the extensions to use their own thread local
  // memory for user-changeable settings. This means you need to use
  // the default field to Bind and can't statically initialize them.
  // The main reasoning to do that is so that the extensions have the
  // values already parsed into their types. If you are setting an int,
  // it does the string parsing once and then when you read it, it is
  // already an int. If we did some shared thing, we would just hand you
  // back the strings and you'd have to parse them on every request or
  // build some convoluted caching mechanism which is slower than just
  // the int access.
  //
  // We could conceivably let you use static memory and have our own
  // thread local here that users can change and then reset it back to
  // the default, but we haven't built that yet.
  //

  IniCallbackData &data =
    use_user ? (*s_user_callbacks)[name] : s_system_ini_callbacks[name];

  data.extension = extension;
  data.mode = mode;
  data.updateCallback = updateCallback;
  data.getCallback = getCallback;
  if (data.iniData == nullptr && userDataCallback != nullptr) {
    data.iniData = userDataCallback();
  }
}

void IniSetting::Unbind(const std::string& name) {
  assert(!name.empty());
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

bool IniSetting::FillInConstant(const std::string& name,
                                const folly::dynamic& value,
                                FollyDynamic) {

  if (config_names_that_use_constants.find(name) ==
      config_names_that_use_constants.end()) {
    return false;
  }
  return IniSetting::Set(name, value, FollyDynamic());
}

bool IniSetting::Set(const std::string& name, const folly::dynamic& value,
                     FollyDynamic) {
  // Need to make sure to update the value if the pair exists already
  // A general insert(make_pair) won't actually update new values.
  bool found = false;
  for (auto& pair : s_system_settings) {
    if (pair.first == name) {
      pair.second = value;
      found = true;
      break;
    }
  }
  if (!found) {
    s_system_settings.insert(make_pair(name, value));
  }
  return ini_set(name, value, PHP_INI_SET_EVERY);
}

bool IniSetting::Set(const String& name, const Variant& value) {
  return Set(name.toCppString(), variant_to_dynamic(value), FollyDynamic());
}

bool IniSetting::SetUser(const std::string& name, const folly::dynamic& value,
                         FollyDynamic) {
  auto it = s_saved_defaults->find(name);
  if (it == s_saved_defaults->end()) {
    folly::dynamic def = nullptr;
    auto success = Get(name, def);
    if (success) {
      s_saved_defaults->insert(make_pair(name, def));
    }
  }
  return ini_set(name, value, PHP_INI_SET_USER);
}

bool IniSetting::SetUser(const String& name, const Variant& value) {
  return SetUser(name.toCppString(), variant_to_dynamic(value), FollyDynamic());
}

bool IniSetting::ResetSystemDefault(const std::string& name) {
  auto it = s_system_settings.find(name);
  if (it == s_system_settings.end()) {
    return false;
  }
  return ini_set(name, it->second, PHP_INI_SET_EVERY);
}

bool IniSetting::GetMode(const std::string& name, Mode& mode) {
  auto cb = get_callback(name);
  if (!cb) {
    return false;
  }
  mode = cb->mode;
  return true;
}

Array IniSetting::GetAll(const String& ext_name, bool details) {
  Array r = Array::Create();

  const Extension* ext = nullptr;
  if (!ext_name.empty()) {
    if (ext_name == s_core) {
      ext = IniSetting::CORE;
    } else {
      ext = ExtensionRegistry::get(ext_name);
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

void add_default_config_files_globbed(
  const char *default_config_file,
  std::function<void (const char *filename)> cb
) {
  glob_t globbuf;
  memset(&globbuf, 0, sizeof(glob_t));
  int flags = 0;  // Use default glob semantics
  int nret = glob(default_config_file, flags, nullptr, &globbuf);
  if (nret == GLOB_NOMATCH ||
      globbuf.gl_pathc == 0 ||
      globbuf.gl_pathv == 0 ||
      nret != 0) {
    globfree(&globbuf);
    return;
  }

  for (int n = 0; n < (int)globbuf.gl_pathc; n++) {
    if (access(globbuf.gl_pathv[n], R_OK) != -1) {
      cb(globbuf.gl_pathv[n]);
    }
  }
  globfree(&globbuf);
}

///////////////////////////////////////////////////////////////////////////////
}
