/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/base/type-variant.h"

#include "hphp/util/hash-map.h"
#include "hphp/util/hash-set.h"

#include <folly/Range.h>

#include <cstdint>
#include <functional>
#include <set>
#include <string>
#include <variant>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Array;
struct Extension;
struct String;
struct StructuredLogEntry;

/**
 * If given an ini setting like "hhvm.abc[def][ghi]=yyy" and we have
 * an ini Variant with the top key being hhvm.abc pointing to its
 * values, we will have something like:
 *       {hhvm.abc {def {ghi : yyy}}}
 * And we pass as the name to get the value of as "def.ghi" for that
 * Variant, we need to iterate over the pointer for the dot (.) values to
 * get to the final setting value of yyy
 */
const IniSettingMap ini_iterate(const IniSettingMap& ini,
                                const std::string& name);

/*
 * Consult the private implementation details in ini-setting.cpp.
 *
 * There's one instance of an IniCallbackData for each initialization
 * setting. Management of system and per-request (per-thread)
 * mappings from names of ini settings to actual IniCallbackData
 * is done privately with the statics s_user_callbacks and
 * s_system_ini_callbacks.
 */

struct IniSettingMap {
  // placeholder to allow the form:
  //    IniSettingMap ini = IniSettingMap::object;
  // is used throughout the codebase. We can convert later to remove them
  enum Type { object };
  IniSettingMap();
  /* implicit */ IniSettingMap(Type);
  /* implicit */ IniSettingMap(const IniSettingMap& i); // copy constructor
  /* implicit */ IniSettingMap(const Variant& v);
  /* implicit */ IniSettingMap(IniSettingMap&& i) noexcept; // move constructor
  IniSettingMap& operator=(const IniSettingMap& i);

public:
  const IniSettingMap operator[](const String& key) const;
  String toString() const { return m_map.toString();}
  Array toArray() const { return m_map.toArray();}
  Object toObject() const { return m_map.toObject();}
  bool isNull() const { return m_map.isNull();}
  bool isString() const { return m_map.isString();}
  bool isArray() const { return m_map.isArray();}
  bool isObject() const { return m_map.isObject();}
  Variant& toVariant() { return m_map; }
  void set(const String& key, const Variant& v);
  TypedValue detach() noexcept {
    return m_map.detach();
  }
private:
  Variant m_map;
};

struct IniSetting {
private:

  struct CallbackData {
    String active_name;
    Variant active_section;
    Variant arr;
  };

public:
  // can remove later in a diff that explicitly changes all uses of
  // IniSetting::Map to IniSettingMap
  using Map = IniSettingMap;
  static const Extension* CORE;
  enum ScannerMode {
    NormalScanner,
    RawScanner,
  };

  enum struct ParserCallbackMode {
    DARRAY = 0,
    DICT = 1,
  };

  struct ParserCallback {
    explicit ParserCallback(ParserCallbackMode mode) : mode_(mode) {}
    virtual ~ParserCallback() {};
    virtual void onSection(const std::string &name, void *arg);
    virtual void onLabel(const std::string &name, void *arg);
    virtual void onEntry(const std::string &key, const std::string &value,
                         void *arg);
    virtual void onPopEntry(const std::string &key, const std::string &value,
                            const std::string &offset, void *arg);
    virtual void onConstant(std::string &result, const std::string &name);
    virtual void onVar(std::string &result, const std::string &name);
    virtual void onOp(std::string &result, char type, const std::string& op1,
                      const std::string& op2);
  protected:
    void makeArray(tv_lval hash, const std::string &offset,
                   const std::string &value);

    Array emptyArrayForMode() const;
    Array& forceToArrayForMode(Variant& var) const;
    Array& forceToArrayForMode(tv_lval var) const;

  private:
    // Substitution copy or symlink via @ or : markers in the config line
    void makeSettingSub(const String &key, const std::string &offset,
                        const std::string &value, Variant& cur_settings);
    void traverseToSet(const String &key, const std::string& offset,
                       tv_lval value, Variant& cur_settings,
                       const std::string& stopChar);

    ParserCallbackMode mode_;
  };
  struct SectionParserCallback : ParserCallback {
    using ParserCallback::ParserCallback;

    void onSection(const std::string& name, void* arg) override;
    void onLabel(const std::string& name, void* arg) override;
    void onEntry(const std::string& key, const std::string& value, void* arg)
        override;
    void onPopEntry(
        const std::string& key,
        const std::string& value,
        const std::string& offset,
        void* arg) override;

   private:
    Variant* activeArray(CallbackData* data);
  };
  struct SystemParserCallback : ParserCallback {
    using ParserCallback::ParserCallback;

    void onEntry(const std::string& key, const std::string& value, void* arg)
        override;
    void onPopEntry(
        const std::string& key,
        const std::string& value,
        const std::string& offset,
        void* arg) override;
    void onConstant(std::string& result, const std::string& name) override;
  };

  enum class Mode {
    Request,  // A value that can be set per request
    Config,   // A value that can be set through configuration
    Constant, // A constant value decided at compile time. Can not be set
  };

public:
  static Variant FromString(const String& ini, const String& filename,
                            bool process_sections = false,
                            int scanner_mode = NormalScanner);
  static IniSettingMap FromStringAsMap(const std::string& ini,
                                       const std::string& filename);

  static bool Get(const std::string& name, std::string &value);
  static bool Get(const String& name, std::string &value);
  static bool Get(const String& name, Variant& value);
  static bool Get(const String& name, String& value);
  static std::string Get(const std::string& name);
  static std::string Get(const String& name);
  static Array GetAll(const String& extension, bool details);
  static std::string GetAllAsJSON();
  static folly::dynamic GetAllAsDynamic();
  static size_t HashAll(const hphp_fast_string_set& toLog,
                        const hphp_fast_string_set& toExclude);
  static void Log(StructuredLogEntry& ent, const hphp_fast_string_set& toLog,
                  const hphp_fast_string_set& toExclude);

  static bool canSet(int64_t settingMode, Mode checkMode) {
    return canSet(static_cast<Mode>(settingMode), checkMode);
  }
  static bool canSet(Mode settingMode, Mode checkMode);
  /**
   * Change an INI setting as if it was in the php.ini file
   */
  static bool SetSystem(const String& name, const Variant& value);
  /**
   * Get a system value
   */
  static bool GetSystem(const String& name, Variant& value);

  /**
   * Change an INI setting as if there was a call to ini_set()
   */
  static bool SetUser(const String& name, const Variant& value);

  /**
   * Restore an INI setting to the default value before the first call to
   * SetUser().
   */
  static void RestoreUser(const String& name);

  /**
   * Fill in constant that may not have been bound when an
   * ini file was initially parsed
   */
   static bool FillInConstant(const std::string& name,
                              const Variant& value);
  /**
   * Get the mode for a setting
   */
  static Optional<Mode> GetMode(const String& name);

#define INI_COMMA ,
#define INI_TYPES(X) \
  X(bool) \
  X(double) \
  INI_TYPES4(X, X, X, X) \
  X(std::string) \
  X(std::vector<uint32_t>) \
  X(std::vector<std::string>) \
  X(std::unordered_map<std::string INI_COMMA int>) \
  X(Array)

#define INI_TYPES4(N, U, M, S) \
  N(char) \
  N(int16_t) \
  N(int32_t) \
  N(int64_t) \
  U(unsigned char) \
  U(uint16_t) \
  U(uint32_t) \
  U(uint64_t) \
  M(std::map<std::string INI_COMMA std::string>) \
  M(std::map<std::string INI_COMMA std::string INI_COMMA stdltistr>) \
  M(hphp_string_imap<std::string>) \
  M(hphp_fast_string_map<std::string>) \
  M(hphp_fast_string_imap<std::string>) \
  S(std::set<std::string>) \
  S(std::set<std::string INI_COMMA stdltistr>) \
  S(boost::container::flat_set<std::string>) \
  S(hphp_fast_string_set)

  template<class T>
  struct SetAndGetImpl {
    SetAndGetImpl(
      std::function<bool (const T&)> setter,
      std::function<T ()> getter,
      T* val = nullptr
    ) : setter(setter)
      , getter(getter)
      , val(val)
    {}
    explicit SetAndGetImpl(T* val) : val(val) {}

    std::function<bool (const T&)> setter{nullptr};
    std::function<T ()> getter{nullptr};
    T* val{nullptr};
  };

  template<class T>
  struct SetAndGet : SetAndGetImpl<std::remove_cv_t<T>> {
    using SetAndGetImpl<std::remove_cv_t<T>>::SetAndGetImpl;
  };

#define F(Ty) SetAndGetImpl<Ty>,
  using OptionData = std::variant<INI_TYPES(F) std::nullptr_t>;
#undef F

  /**
   * The heavy lifting of creating ini settings. First of all, if you don't
   * have to use this method, please don't. Instead use the simpler:
   *
   *   IniSetting::Bind(this, IniSetting::Mode::Config, "my.ini", &some_global_var);
   *     or
   *   IniSetting::Bind(this, IniSetting::Mode::Request, "my.ini", &some_thread_local_var);
   *
   * If you have to do something special before your variable is set or gotten
   * then you can use this function to add callbacks. Both callbacks are
   * optional (but if you don't pass any, why are you using this method in the
   * first place?). If the setter callback returns "false" then the value will
   * not be saved into p. If the getter is not provided, the contents of p will
   * directly be used.
   */
  template<class T>
  static void Bind(const Extension* extension, const Mode mode,
                   const std::string& name, const char *defaultValue,
                   SetAndGetImpl<T> callbacks) {
    Bind(extension, mode, name, OptionData(callbacks), defaultValue);
  }

  template<class T>
  static void Bind(const Extension* extension, const Mode mode,
                   const std::string& name, SetAndGet<T> callbacks) {
    Bind(extension, mode, name, nullptr, callbacks);
  }

  /**
   * Prefer to use this method whenever possible (the non-default one is ok
   * too). Use Config::Bind if immediate access to the ini setting is
   * necessary. For performance reasons, Config::Bind should only be used
   * when access to the ini setting is needed prior to loading the rest of
   * the ini settings.
   */
  template<class T>
  static void Bind(const Extension* extension, const Mode mode,
                   const std::string& name, const char *defaultValue, T *p) {
    Bind(extension, mode, name, defaultValue, SetAndGet<T>(p));
  }

  template<class T>
  static void Bind(const Extension* extension, const Mode mode,
                   const std::string& name, T *p) {
    Bind(extension, mode, name, nullptr, p);
  }

  static void Unbind(const std::string& name);

  /**
   * Set an ini setting back to the value from the config file
   * (or the hard-coded default)
   */
  static bool ResetSystemDefault(const std::string& name);

  /**
   * After a request, we want to set all user settings back to their original
   * defaults before the request. This should be called on requestShutdown.
   */
  static void ResetSavedDefaults();

  /**
   *  Used to help us late bind extension constants (e.g. E_ALL) that
   *  were incorrectly bound initially, and needed to be bound again after
   *  all was loaded.
   */
  static bool s_config_is_a_constant;
  static std::set<std::string> config_names_that_use_constants;

  /**
   * A flag to ensure we don't try to add a system setting after the
   * runtime options have been loaded
   */
  static bool s_system_settings_are_set;

private:
  static void Bind(
    const Extension* extension,
    const Mode mode,
    const std::string& name,
    OptionData callbacks,
    const char* defaultValue);
};

int64_t convert_bytes_to_long(folly::StringPiece value);
std::string convert_long_to_bytes(int64_t value);

void add_default_config_files_globbed(
  const char *default_config_file,
  std::function<void (const char *filename)> cb);

#define F(Ty) \
  Variant ini_get(Ty&); \
  bool ini_on_update(const Variant&, Ty&);

INI_TYPES(F)

#undef F

///////////////////////////////////////////////////////////////////////////////
}
