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

std::string hdfToIni(const std::string& name) {
  std::string out = "hhvm.";
  size_t idx = 0;
  for (auto& c : name) {
    // This is the first or last character
    if (idx == 0 || idx == name.length() - 1) {
      out += tolower(c);
    } else if (!isalpha(c)) {
      // Any . or _ or numeral is just output with no special behavior
      out += c;
    } else {
      if (isupper(c) && isupper(name[idx - 1 ]) && islower(name[idx + 1])) {
      // Handle something like "SSLPort", and c = "P", which will then put
      // the underscore between the "L" and "P"
        out += "_";
        out += tolower(c);
      } else if (islower(c) && isupper(name[idx + 1])) {
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

  return out;
}

std::string Config::IniName(const Hdf& config) {
  return Config::IniName(config.getFullPath());
}

std::string Config::IniName(const std::string& config) {
  return hdfToIni(config);
}

void Config::ParseIniString(const std::string iniStr, IniSetting::Map &ini) {
  Config::SetParsedIni(ini, iniStr, "", false);
}

void Config::ParseHdfString(const std::string hdfStr, Hdf &hdf,
                            IniSetting::Map &ini) {
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
    Config::ParseHdfFile(filename, hdf, ini);
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

void Config::ParseHdfFile(const std::string &filename, Hdf &hdf,
                          IniSetting::Map &ini) {
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
                         const char *defValue /* = nullptr */) {
  auto* value = ini.get_ptr(IniName(config));
  if (value && value->isString()) {
    return value->data();
  }
  return config.configGet(defValue);
}

template<class T> static T variant_init(T v) {
    return v;
}
static int64_t variant_init(uint32_t v) {
    return v;
}

#define CONFIG_BODY(T, METHOD) \
T Config::Get##METHOD(const IniSetting::Map &ini, const Hdf& config, \
                      const T defValue /* = 0ish */) { \
  auto* value = ini.get_ptr(IniName(config)); \
  if (value && value->isString()) { \
    T ini_ret, hdf_ret; \
    ini_on_update(value->data(), ini_ret); \
    /* The HDF still wins because the -v options
     * are still done via HDF, for now.*/ \
    hdf_ret = config.configGet##METHOD(ini_ret); \
    if (ini_ret != hdf_ret) { \
      IniSetting::Set(IniName(config), variant_init(hdf_ret)); \
    } \
    return hdf_ret; \
  } \
  return config.configGet##METHOD(defValue); \
} \
void Config::Bind(T& loc, const IniSetting::Map &ini, const Hdf& config, \
                  const T defValue /* = 0ish */) { \
  loc = Get##METHOD(ini, config, defValue); \
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM, \
                   IniName(config), &loc); \
} \
void Config::Bind(T& loc, const IniSetting::Map &ini, std::string name, \
                  const T defValue /* = 0ish */) { \
  auto* value = ini.get_ptr(name); \
  if (value && value->isString()) { \
    ini_on_update(value->data(), loc); \
  } else { \
    loc = defValue; \
  } \
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM, \
                   name, &loc); \
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

static HackStrictOption GetHackStrictOption(const IniSettingMap& ini,
                                            const Hdf& config) {
  auto val = Config::GetString(ini, config);
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
                  const Hdf& config) {
  // Currently this doens't bind to ini_get since it is hard to thread through
  // an enum
  loc = GetHackStrictOption(ini, config);
}

void Config::Bind(std::vector<std::string>& loc, const IniSettingMap& ini,
                  const Hdf& config) {
  std::vector<std::string> ret;
  auto ini_name = IniName(config);
  auto* value = ini.get_ptr(ini_name);
  if (value && value->isObject()) {
    ini_on_update(*value, ret);
    loc = ret;
  }
  // If there is an HDF setting for the config, then it still wins for
  // the RuntimeOption value until we obliterate HDFs
  ret.clear();
  config.configGet(ret);
  if (ret.size() > 0) {
    loc = ret;
  }
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM, ini_name,
                   &loc);
}

void Config::Bind(std::map<std::string, std::string>& loc,
                  const IniSettingMap& ini, const Hdf& config) {
  std::map<std::string, std::string> ret;
  auto ini_name = IniName(config);
  auto* value = ini.get_ptr(ini_name);
  if (value && value->isObject()) {
    ini_on_update(*value, ret);
    loc = ret;
  }
  // If there is an HDF setting for the config, then it still wins for
  // the RuntimeOption value until we obliterate HDFs
  ret.clear();
  config.configGet(ret);
  if (ret.size() > 0) {
    loc = ret;
  }
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM, ini_name,
                   &loc);
}

}
