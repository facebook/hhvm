/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_CONFIG_H_
#define incl_HPHP_CONFIG_H_

#include "folly/dynamic.h"
#include "hphp/util/hdf.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

typedef folly::dynamic IniSettingMap;

/**
 * Parts of the language can individually be made stricter, warning or
 * erroring when there's dangerous/unintuive usage; for example,
 * array_fill_keys() with non-int/string keys: Hack.Lang.StrictArrayFillKeys
 */
enum class HackStrictOption {
  OFF, // PHP5 behavior
  WARN,
  ERROR
};

struct Config {

  static void Parse(const std::string &config, IniSettingMap &ini, Hdf &hdf);

  /** Prefer the Bind() over the GetFoo() as it makes ini_get() work too. */
  static void Bind(bool& loc, const IniSettingMap &ini,
                   const Hdf& config, const bool defValue = false);
  static void Bind(const char*& loc, const IniSettingMap &ini,
                   const Hdf& config, const char *defValue = nullptr);
  static void Bind(std::string& loc, const IniSettingMap &ini,
                   const Hdf& config, const std::string defValue = "");
  static void Bind(char& loc, const IniSettingMap &ini,
                   const Hdf& config, const char defValue = 0);
  static void Bind(unsigned char& loc,const IniSettingMap &ini,
                   const Hdf& config, const unsigned char defValue = 0);
  static void Bind(int16_t& loc, const IniSettingMap &ini,
                   const Hdf& config, const int16_t defValue = 0);
  static void Bind(uint16_t& loc, const IniSettingMap &ini,
                   const Hdf& config, const uint16_t defValue = 0);
  static void Bind(int32_t& loc, const IniSettingMap &ini,
                   const Hdf& config, const int32_t defValue = 0);
  static void Bind(uint32_t& loc, const IniSettingMap &ini,
                   const Hdf& config, const uint32_t defValue = 0);
  static void Bind(int64_t& loc, const IniSettingMap &ini,
                   const Hdf& config, const int64_t defValue = 0);
  static void Bind(uint64_t& loc, const IniSettingMap &ini,
                   const Hdf& config, const uint64_t defValue = 0);
  static void Bind(double& loc, const IniSettingMap &ini,
                   const Hdf& config, const double defValue = 0);
  static void Bind(HackStrictOption& loc, const IniSettingMap &ini,
                   const Hdf& config);
  static void Bind(std::vector<std::string>& loc, const IniSettingMap& ini,
                   const Hdf& config);

  /**
   * These Bind()s should be used for ini settings. Specifically, they should
   * be used when the bound setting is needed before the main ini processing
   * pass. Unlike IniSetting::Bind, these bindings will fetch the value in
   * an ini setting if it is set otherwise it will use the defValue.
   */
  static void Bind(bool& loc, const IniSettingMap &ini,
                   const std::string name, const bool defValue = false);
  static void Bind(const char*& loc, const IniSettingMap &ini,
                   const std::string name, const char *defValue = nullptr);
  static void Bind(std::string& loc, const IniSettingMap &ini,
                   const std::string name, const std::string defValue = "");
  static void Bind(char& loc, const IniSettingMap &ini,
                   const std::string name, const char defValue = 0);
  static void Bind(unsigned char& loc,const IniSettingMap &ini,
                   const std::string name, const unsigned char defValue = 0);
  static void Bind(int16_t& loc, const IniSettingMap &ini,
                   const std::string name, const int16_t defValue = 0);
  static void Bind(uint16_t& loc, const IniSettingMap &ini,
                   const std::string name, const uint16_t defValue = 0);
  static void Bind(int32_t& loc, const IniSettingMap &ini,
                   const std::string name, const int32_t defValue = 0);
  static void Bind(uint32_t& loc, const IniSettingMap &ini,
                   const std::string name, const uint32_t defValue = 0);
  static void Bind(int64_t& loc, const IniSettingMap &ini,
                   const std::string name, const int64_t defValue = 0);
  static void Bind(uint64_t& loc, const IniSettingMap &ini,
                   const std::string name, const uint64_t defValue = 0);
  static void Bind(double& loc, const IniSettingMap &ini,
                   const std::string name, const double defValue = 0);


  static bool GetBool(const IniSettingMap &ini, const Hdf& config,
                      const bool defValue = false);
  static const char *Get(const IniSettingMap &ini, const Hdf& config,
                         const char *defValue = nullptr);
  static std::string GetString(const IniSettingMap &ini, const Hdf& config,
                               const std::string defValue = "");
  static char GetByte(const IniSettingMap &ini, const Hdf& config,
                      const char defValue = 0);
  static unsigned char GetUByte(const IniSettingMap &ini, const Hdf& config,
                                const unsigned char defValue = 0);
  static int16_t GetInt16(const IniSettingMap &ini, const Hdf& config,
                          const int16_t defValue = 0);
  static uint16_t GetUInt16(const IniSettingMap &ini, const Hdf& config,
                            const uint16_t defValue = 0);
  static int32_t GetInt32(const IniSettingMap &ini, const Hdf& config,
                          const int32_t defValue = 0);
  static uint32_t GetUInt32(const IniSettingMap &ini, const Hdf& config,
                            const uint32_t defValue = 0);
  static int64_t GetInt64(const IniSettingMap &ini, const Hdf& config,
                          const int64_t defValue = 0);
  static uint64_t GetUInt64(const IniSettingMap &ini, const Hdf& config,
                            const uint64_t defValue = 0);
  static double GetDouble(const IniSettingMap &ini, const Hdf& config,
                          const double defValue = 0);

  template<class T>
  static void Get(const IniSettingMap &ini, const Hdf& config, T &data) {
    config.configGet(data);
    if (!data.empty()) {
      return;
    }
    auto key = IniName(config);
    auto* value = ini.get_ptr(key);
    if (!value) {
      return;
    }
    if (value->isArray() || value->isObject()) {
      for (auto &pair : value->items()) {
        StringInsert(data, pair.first.asString().toStdString(),
                           pair.second.asString().toStdString());
      }
    }
  }

  private:

  static std::string IniName(const Hdf& config);

  static void StringInsert(std::vector<std::string> &values,
                           const std::string &key,
                           const std::string &value) {
    values.push_back(value);
  }
  static void StringInsert(boost::container::flat_set<std::string> &values,
                           const std::string &key,
                           const std::string &value) {
    values.insert(value);
  }
  static void StringInsert(std::set<std::string, stdltistr> &values,
                           const std::string &key,
                           const std::string &value) {
    values.insert(value);
  }
  static void StringInsert(std::set<std::string> &values,
                           const std::string &key,
                           const std::string &value) {
    values.insert(value);
  }
  static void StringInsert(std::map<std::string, std::string> &values,
                           const std::string &key,
                           const std::string &value) {
    values[key] = value;
  }
  static void StringInsert(hphp_string_imap<std::string> &values,
                           const std::string &key,
                           const std::string &value) {
    values[key] = value;
  }
};

}

#endif /* incl_HPHP_CONFIG_H_ */
