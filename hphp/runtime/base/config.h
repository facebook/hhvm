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

#include <folly/dynamic.h>
#include "hphp/util/hdf.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

typedef folly::dynamic IniSettingMap;
typedef std::vector<std::string> ConfigVector;
typedef std::map<std::string, std::string> ConfigMap;
typedef std::set<std::string> ConfigSet;
// with comparer
typedef std::set<std::string, stdltistr> ConfigSetC;
typedef std::map<std::string, std::string, stdltistr> ConfigMapC;
typedef boost::container::flat_set<std::string> ConfigFlatSet;
typedef hphp_string_imap<std::string> ConfigIMap;

/**
 * Parts of the language can individually be made stricter, warning or
 * erroring when there's dangerous/unintuive usage; for example,
 * array_fill_keys() with non-int/string keys: Hack.Lang.StrictArrayFillKeys
 */
enum class HackStrictOption {
  OFF, // PHP5 behavior
  WARN,
  ON
};


struct Config {
  /*
   * Normalizes hdf string names to their ini counterparts
   *
   * We have special handling for a few hdf strings such as those containing
   * MySQL, Eval, IPv[4|6] and EnableHipHopSyntax
   */
  static std::string IniName(const Hdf& config,
                             const bool prepend_hhvm = true);
  static std::string IniName(const std::string& config,
                             const bool prepend_hhvm = true);

  static void ParseConfigFile(const std::string &filename, IniSettingMap &ini,
                              Hdf &hdf);

  static void ParseIniFile(const std::string &filename);
  static void ParseIniFile(const std::string &filename, IniSettingMap &ini,
                           const bool constants_only = false);

  static void ParseHdfFile(const std::string &filename, Hdf &hdf);

  // Parse and process a .ini string (e.g., -d)
  static void ParseIniString(const std::string iniStr, IniSettingMap &ini);

  // Parse and process a .hdf string (e.g., -v)
  static void ParseHdfString(const std::string hdfStr, Hdf &hdf);

  /**
   * Prefer the Bind() over the GetFoo() as it makes ini_get() work too.
   * These Bind()s should be used for ini settings. Specifically, they
   * should be used when the bound setting is needed before the main ini
   * processing pass. Unlike IniSetting::Bind, these bindings will fetch the
   * value in an ini setting if it is set otherwise it will use the defValue.
   */
  static void Bind(bool& loc, const IniSettingMap &ini,
                   const Hdf& config, const std::string& name = "",
                   const bool defValue = false,
                   const bool prepend_hhvm = true);
  static void Bind(const char*& loc, const IniSettingMap &ini,
                   const Hdf& config, const std::string& name = "",
                   const char *defValue = nullptr,
                   const bool prepend_hhvm = true);
  static void Bind(std::string& loc, const IniSettingMap &ini,
                   const Hdf& config, const std::string& name = "",
                   const std::string defValue = "",
                   const bool prepend_hhvm = true);
  static void Bind(char& loc, const IniSettingMap &ini,
                   const Hdf& config, const std::string& name = "",
                   const char defValue = 0, const bool prepend_hhvm = true);
  static void Bind(unsigned char& loc,const IniSettingMap &ini,
                   const Hdf& config, const std::string& name = "",
                   const unsigned char defValue = 0,
                   const bool prepend_hhvm = true);
  static void Bind(int16_t& loc, const IniSettingMap &ini,
                   const Hdf& config, const std::string& name = "",
                   const int16_t defValue = 0,
                   const bool prepend_hhvm = true);
  static void Bind(uint16_t& loc, const IniSettingMap &ini,
                   const Hdf& config, const std::string& name = "",
                   const uint16_t defValue = 0,
                   const bool prepend_hhvm = true);
  static void Bind(int32_t& loc, const IniSettingMap &ini,
                   const Hdf& config, const std::string& name = "",
                   const int32_t defValue = 0,
                   const bool prepend_hhvm = true);
  static void Bind(uint32_t& loc, const IniSettingMap &ini,
                   const Hdf& config, const std::string& name = "",
                   const uint32_t defValue = 0,
                   const bool prepend_hhvm = true);
  static void Bind(int64_t& loc, const IniSettingMap &ini,
                   const Hdf& config, const std::string& name = "",
                   const int64_t defValue = 0,
                   const bool prepend_hhvm = true);
  static void Bind(uint64_t& loc, const IniSettingMap &ini,
                   const Hdf& config, const std::string& name = "",
                   const uint64_t defValue = 0,
                   const bool prepend_hhvm = true);
  static void Bind(double& loc, const IniSettingMap &ini,
                   const Hdf& config, const std::string& name = "",
                   const double defValue = 0,
                   const bool prepend_hhvm = true);
  static void Bind(HackStrictOption& loc, const IniSettingMap &ini,
                   const Hdf& config, const std::string& name = "");
  static void Bind(ConfigVector& loc, const IniSettingMap& ini,
                   const Hdf& config, const std::string& name = "",
                   const ConfigVector& defValue = ConfigVector(),
                   const bool prepend_hhvm = true);
  static void Bind(ConfigMap& loc, const IniSettingMap& ini, const Hdf& config,
                   const std::string& name = "",
                   const ConfigMap& defValue = ConfigMap(),
                   const bool prepend_hhvm = true);
  static void Bind(ConfigMapC& loc, const IniSettingMap& ini, const Hdf& config,
                   const std::string& name = "",
                   const ConfigMapC& defValue = ConfigMapC(),
                   const bool prepend_hhvm = true);
  static void Bind(ConfigSet& loc, const IniSettingMap& ini, const Hdf& config,
                   const std::string& name = "",
                   const ConfigSet& defValue = ConfigSet(),
                   const bool prepend_hhvm = true);
  static void Bind(ConfigSetC& loc, const IniSettingMap& ini, const Hdf& config,
                   const std::string& name = "",
                   const ConfigSetC& defValue = ConfigSetC(),
                   const bool prepend_hhvm = true);
  static void Bind(ConfigIMap& loc, const IniSettingMap& ini, const Hdf& config,
                   const std::string& name = "",
                   const ConfigIMap& defValue = ConfigIMap(),
                   const bool prepend_hhvm = true);
  static void Bind(ConfigFlatSet& loc, const IniSettingMap& ini,
                   const Hdf& config, const std::string& name = "",
                   const ConfigFlatSet& defValue = ConfigFlatSet(),
                   const bool prepend_hhvm = true);

  /**
   * Master Get Methods to get values associated with an ini or hdf setting.
   * These methods just get the value. They do not bind to a variable for
   * enabling ini_get()
   */
  static bool GetBool(const IniSettingMap &ini, const Hdf& config,
                      const std::string& name = "",
                      const bool defValue = false,
                      const bool prepend_hhvm = true);
  static const char *Get(const IniSettingMap &ini, const Hdf& config,
                         const std::string& name = "",
                         const char *defValue = nullptr,
                         const bool prepend_hhvm = true);
  static std::string GetString(const IniSettingMap &ini, const Hdf& config,
                               const std::string& name = "",
                               const std::string defValue = "",
                               const bool prepend_hhvm = true);
  static char GetByte(const IniSettingMap &ini, const Hdf& config,
                      const std::string& name = "", const char defValue = 0,
                      const bool prepend_hhvm = true);
  static unsigned char GetUByte(const IniSettingMap &ini, const Hdf& config,
                                const std::string& name = "",
                                const unsigned char defValue = 0,
                                const bool prepend_hhvm = true);
  static int16_t GetInt16(const IniSettingMap &ini, const Hdf& config,
                          const std::string& name = "",
                          const int16_t defValue = 0,
                          const bool prepend_hhvm = true);
  static uint16_t GetUInt16(const IniSettingMap &ini, const Hdf& config,
                            const std::string& name = "",
                            const uint16_t defValue = 0,
                            const bool prepend_hhvm = true);
  static int32_t GetInt32(const IniSettingMap &ini, const Hdf& config,
                          const std::string& name = "",
                          const int32_t defValue = 0,
                          const bool prepend_hhvm = true);
  static uint32_t GetUInt32(const IniSettingMap &ini, const Hdf& config,
                            const std::string& name = "",
                            const uint32_t defValue = 0,
                            const bool prepend_hhvm = true);
  static int64_t GetInt64(const IniSettingMap &ini, const Hdf& config,
                          const std::string& name = "",
                          const int64_t defValue = 0,
                          const bool prepend_hhvm = true);
  static uint64_t GetUInt64(const IniSettingMap &ini, const Hdf& config,
                            const std::string& name = "",
                            const uint64_t defValue = 0,
                            const bool prepend_hhvm = true);
  static double GetDouble(const IniSettingMap &ini, const Hdf& config,
                          const std::string& name = "",
                          const double defValue = 0,
                          const bool prepend_hhvm = true);
  static ConfigVector GetVector(const IniSettingMap& ini, const Hdf& config,
                                const std::string& name = "",
                                const ConfigVector& defValue = ConfigVector(),
                                const bool prepend_hhvm = true);
  static ConfigMap GetMap(const IniSettingMap& ini, const Hdf& config,
                          const std::string& name = "",
                          const ConfigMap& defValue = ConfigMap(),
                          const bool prepend_hhvm = true);
  static ConfigMapC GetMapC(const IniSettingMap& ini, const Hdf& config,
                          const std::string& name = "",
                          const ConfigMapC& defValue = ConfigMapC(),
                          const bool prepend_hhvm = true);
  static ConfigSet GetSet(const IniSettingMap& ini, const Hdf& config,
                          const std::string& name = "",
                          const ConfigSet& defValue = ConfigSet(),
                          const bool prepend_hhvm = true);
  static ConfigSetC GetSetC(const IniSettingMap& ini, const Hdf& config,
                            const std::string& name = "",
                            const ConfigSetC& defValue = ConfigSetC(),
                            const bool prepend_hhvm = true);
  static ConfigIMap GetIMap(const IniSettingMap& ini, const Hdf& config,
                            const std::string& name = "",
                            const ConfigIMap& defValue = ConfigIMap(),
                            const bool prepend_hhvm = true);
  static ConfigFlatSet GetFlatSet(const IniSettingMap& ini, const Hdf& config,
                                  const std::string& name = "",
                                  const ConfigFlatSet& defValue
                                    = ConfigFlatSet(),
                                  const bool prepend_hhvm = true);

  /**
   * Use the Iterate method for iterating over options that are stored as
   * objects in runtime options (e.g. FilesMatch). This function iterates over
   * the settings passed as ini/hdf, calls back to, generally, the constructor
   * of the object in question.
   *
   * Note: For now, we are not `ini_get()` enabling these type of options as
   * it is not trivial to come up with a non-hacky and workable way to store
   * the data correctly. Also, as usual, Hdf takes priority.
   */
  static void Iterate(std::function<void (const IniSettingMap&,
                                          const Hdf&,
                                          const std::string&)> cb,
                      const IniSettingMap &ini, const Hdf& config,
                      const std::string &name, const bool prepend_hhvm = true);

  private:

  static void SetParsedIni(IniSettingMap &ini, const std::string confStr,
                           const std::string filename, bool extensions_only);

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
  static void StringInsert(std::map<std::string, std::string,
                           stdltistr> &values,
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
