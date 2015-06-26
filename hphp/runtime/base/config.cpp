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

#include "hphp/runtime/base/config.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <fstream>

#include "hphp/compiler/option.h"
#include "hphp/runtime/base/ini-setting.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

std::string Config::IniName(const Hdf& config,
                            bool prepend_hhvm /* = true */) {
  return Config::IniName(config.getFullPath());
}

std::string Config::IniName(const std::string& config,
                            bool prepend_hhvm /* = true */) {
  std::string out = "";
  if (prepend_hhvm) {
    out += "hhvm.";
  }
  size_t idx = 0;
  for (auto& c : config) {
    // This is the first or last character
    if (idx == 0 || idx == config.length() - 1) {
      out += tolower(c);
    } else if (!isalpha(c)) {
      // Any . or _ or numeral is just output with no special behavior
      out += c;
    } else {
      if (isupper(c) && isupper(config[idx - 1 ]) && islower(config[idx + 1])) {
      // Handle something like "SSLPort", and c = "P", which will then put
      // the underscore between the "L" and "P"
        out += "_";
        out += tolower(c);
      } else if (islower(c) && isupper(config[idx + 1])) {
      // Handle something like "PathDebug", and c = "h", which will then put
      // the underscore between the "h" and "D"
        out += tolower(c);
        out += "_";
      } else {
      // Otherwise we just output as lower
        out += tolower(c);
      }
    }
    idx++;
  }

  boost::replace_first(out,
                       "hhvm.server.upload.max_file_uploads",
                       "max_file_uploads");
  // Make sure IPv6 or IPv4 are handled correctly
  boost::replace_first(out, "_i_pv", "_ipv");
  boost::replace_first(out, ".i_pv", ".ipv");
  // urls are special too. Let's not have "ur_ls"
  boost::replace_first(out, "_ur_ls", "_urls");
  boost::replace_first(out, ".ur_ls", ".urls");
  // No use of Eval in our ini strings
  boost::replace_first(out, ".eval.", ".");
  boost::replace_first(out, ".my_sql.", ".mysql.");
  boost::replace_first(out, ".enable_hip_hop_syntax", ".force_hh");

  // Fix "XDebug" turning into "x_debug".
  boost::replace_first(out, "hhvm.debugger.x_debug_", "xdebug.");

  return out;
}

void Config::ParseIniString(const std::string iniStr, IniSetting::Map &ini) {
  Config::SetParsedIni(ini, iniStr, "", false);
}

void Config::ParseHdfString(const std::string hdfStr, Hdf &hdf) {
  hdf.fromString(hdfStr.c_str());
}

void Config::ParseConfigFile(const std::string &filename, IniSetting::Map &ini,
                             Hdf &hdf) {
  // We don't allow a filename of just ".ini"
  if (boost::ends_with(filename, ".ini") && filename.length() > 4) {
    Config::ParseIniFile(filename, ini);
  } else {
    // For now, assume anything else is an hdf file
    // TODO(#5151773): Have a non-invasive warning if HDF file does not end
    // .hdf
    Config::ParseHdfFile(filename, hdf);
  }
}

void Config::ParseIniFile(const std::string &filename) {
  IniSetting::Map ini = IniSetting::Map::object;;
  Config::ParseIniFile(filename, ini, false);
}

void Config::ParseIniFile(const std::string &filename, IniSetting::Map &ini,
                          const bool constants_only /* = false */) {
    std::ifstream ifs(filename);
    const std::string str((std::istreambuf_iterator<char>(ifs)),
                          std::istreambuf_iterator<char>());
    Config::SetParsedIni(ini, str, filename, constants_only);
}

void Config::ParseHdfFile(const std::string &filename, Hdf &hdf) {
  hdf.append(filename);
}

void Config::SetParsedIni(IniSetting::Map &ini, const std::string confStr,
                          const std::string filename, bool constants_only) {
  assert(ini != nullptr);
  auto parsed_ini = IniSetting::FromStringAsMap(confStr, filename);
  for (auto &pair : parsed_ini.items()) {
    ini[pair.first] = pair.second;
    if (constants_only) {
      IniSetting::FillInConstant(pair.first.data(), pair.second,
                                 IniSetting::FollyDynamic());
    } else {
      IniSetting::Set(pair.first.data(), pair.second,
                      IniSetting::FollyDynamic());
    }
  }
}

const char* Config::Get(const IniSetting::Map &ini, const Hdf& config,
                        const std::string& name /* = "" */,
                        const char *defValue /* = nullptr */,
                        const bool prepend_hhvm /* = true */) {
  auto ini_name = IniName(name, prepend_hhvm);
  Hdf hdf = name != "" ? config[name] : config;
  auto* value = ini_iterate(ini, ini_name);
  if (value && value->isString()) {
    // See generic Get##METHOD below for why we are doing this
    const char* ini_ret = value->data();
    const char* hdf_ret = hdf.configGet(value->data());
    if (hdf_ret != ini_ret) {
      ini_ret = hdf_ret;
      IniSetting::Set(ini_name, ini_ret);
    }
    return ini_ret;
  }
  return hdf.configGet(defValue);
}

template<class T> static T variant_init(T v) {
    return v;
}
static int64_t variant_init(uint32_t v) {
    return v;
}

#define CONFIG_BODY(T, METHOD) \
T Config::Get##METHOD(const IniSetting::Map &ini, const Hdf& config, \
                      const std::string &name /* = "" */, \
                      const T defValue /* = 0ish */, \
                      const bool prepend_hhvm /* = true */) { \
  auto ini_name = IniName(name, prepend_hhvm); \
  /* If we don't pass a name, then we just use the raw config as-is. */ \
  /* This could happen when we are at a known leaf of a config node. */ \
  Hdf hdf = name != "" ? config[name] : config; \
  auto* value = ini_iterate(ini, ini_name); \
  if (value && value->isString()) { \
    T ini_ret, hdf_ret; \
    ini_on_update(value->data(), ini_ret); \
    /* I don't care what the ini_ret was if it isn't equal to what  */ \
    /* is returned back from from an HDF get call, which it will be */ \
    /* if the call just passes back ini_ret because either they are */ \
    /* the same or the hdf option associated with this name does    */ \
    /* not exist.... REMEMBER HDF WINS OVER INI UNTIL WE WIPE HDF   */ \
    hdf_ret = hdf.configGet##METHOD(ini_ret); \
    if (hdf_ret != ini_ret) { \
      ini_ret = hdf_ret; \
      IniSetting::Set(ini_name, variant_init(ini_ret)); \
    } \
    return ini_ret; \
  } \
  /* If there is a value associated with this setting in the hdf config */ \
  /* then return it; otherwise the defValue will be returned as it is   */ \
  /* assigned to the return value for this call when nothing exists     */ \
  return hdf.configGet##METHOD(defValue); \
} \
void Config::Bind(T& loc, const IniSetting::Map &ini, const Hdf& config, \
                  const std::string& name /* = "" */, \
                  const T defValue /* = 0ish */, \
                  const bool prepend_hhvm /* = true */) { \
  loc = Get##METHOD(ini, config, name, defValue, prepend_hhvm); \
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM, \
                   IniName(name, prepend_hhvm), &loc); \
}

CONFIG_BODY(bool, Bool)
CONFIG_BODY(char, Byte)
CONFIG_BODY(unsigned char, UByte)
CONFIG_BODY(int16_t, Int16)
CONFIG_BODY(uint16_t, UInt16)
CONFIG_BODY(int32_t, Int32)
CONFIG_BODY(uint32_t, UInt32)
CONFIG_BODY(int64_t, Int64)
CONFIG_BODY(uint64_t, UInt64)
CONFIG_BODY(double, Double)
CONFIG_BODY(std::string, String)

#define CONTAINER_CONFIG_BODY(T, METHOD) \
T Config::Get##METHOD(const IniSetting::Map& ini, const Hdf& config, \
                      const std::string& name /* = "" */, \
                      const T& defValue /* = T() */, \
                      const bool prepend_hhvm /* = true */) { \
  auto ini_name = IniName(name, prepend_hhvm); \
  Hdf hdf = name != "" ? config[name] : config; \
  T ini_ret, hdf_ret; \
  const folly::dynamic* value = ini_iterate(ini, ini_name); \
  if (value && (value->isArray() || value->isObject())) { \
    ini_on_update(*value, ini_ret); \
    /** Make sure that even if we have an ini value, that if we also **/ \
    /** have an hdf value, that it maintains its edge as beating out **/ \
    /** ini                                                          **/ \
    if (hdf.exists() && !hdf.isEmpty()) { \
      hdf.configGet(hdf_ret); \
      if (hdf_ret != ini_ret) { \
        ini_ret = hdf_ret; \
        IniSetting::Set(ini_name, ini_get(ini_ret), \
                        IniSetting::FollyDynamic()); \
      } \
    } \
    return ini_ret; \
  } \
  if (hdf.exists() && !hdf.isEmpty()) { \
    hdf.configGet(hdf_ret); \
    return hdf_ret; \
  } \
  return defValue; \
} \
void Config::Bind(T& loc, const IniSetting::Map& ini, const Hdf& config, \
                  const std::string& name /* = "" */, \
                  const T& defValue /* = T() */, \
                  const bool prepend_hhvm /* = true */) { \
  loc = Get##METHOD(ini, config, name, defValue, prepend_hhvm); \
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM, \
                   IniName(name, prepend_hhvm), &loc); \
}

CONTAINER_CONFIG_BODY(ConfigVector, Vector)
CONTAINER_CONFIG_BODY(ConfigMap, Map)
CONTAINER_CONFIG_BODY(ConfigMapC, MapC)
CONTAINER_CONFIG_BODY(ConfigSet, Set)
CONTAINER_CONFIG_BODY(ConfigSetC, SetC)
CONTAINER_CONFIG_BODY(ConfigFlatSet, FlatSet)
CONTAINER_CONFIG_BODY(ConfigIMap, IMap)

static HackStrictOption GetHackStrictOption(const IniSettingMap& ini,
                                            const Hdf& config,
                                            const std::string& name /* = "" */
                                           ) {
  auto val = Config::GetString(ini, config, name);
  if (val.empty()) {
    if (Option::EnableHipHopSyntax || RuntimeOption::EnableHipHopSyntax) {
      return HackStrictOption::ON;
    }
    return HackStrictOption::OFF;
  }
  if (val == "warn") {
    return HackStrictOption::WARN;
  }
  bool ret;
  ini_on_update(val, ret);
  return ret ? HackStrictOption::ON : HackStrictOption::OFF;
}

void Config::Bind(HackStrictOption& loc, const IniSettingMap& ini,
                  const Hdf& config, const std::string& name /* = "" */) {
  // Currently this doens't bind to ini_get since it is hard to thread through
  // an enum
  loc = GetHackStrictOption(ini, config, name);
}

// No `ini` binding yet. Hdf still takes precedence but will be removed
// once we have made all options ini-aware. All new settings should
// use the ini path of this method (i.e., pass a bogus Hdf or keep it null)
void Config::Iterate(std::function<void (const IniSettingMap&,
                                         const Hdf&,
                                         const std::string&)> cb,
                     const IniSettingMap &ini, const Hdf& config,
                     const std::string &name,
                     const bool prepend_hhvm /* = true */) {
  // We shouldn't be passing a leaf here. That's why name is not
  // optional.
  assert(!name.empty());
  Hdf hdf = config[name];
  if (hdf.exists() && !hdf.isEmpty()) {
    for (Hdf c = hdf.firstChild(); c.exists(); c = c.next()) {
      cb(IniSetting::Map::object, c, "");
    }
  } else {
    Hdf empty;
    auto ini_name = IniName(name, prepend_hhvm);
    auto* ini_value = ini_iterate(ini, ini_name);
    if (ini_value && ini_value->isObject()) {
      for (auto& pair : ini_value->items()) {
        cb(pair.second, empty, pair.first.data());
      }
    }
  }
}

}
