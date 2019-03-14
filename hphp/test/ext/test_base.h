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

#ifndef incl_HPHP_TEST_BASE_H_
#define incl_HPHP_TEST_BASE_H_

#include "hphp/test/ext/test.h"
#include "hphp/util/text-util.h"

#include <cassert>
#include <exception>
#include <string>

using namespace HPHP;
///////////////////////////////////////////////////////////////////////////////

struct TestBase {
  TestBase();
  virtual ~TestBase() {}

  virtual bool RunTests(const std::string &which) = 0;

  int fail_count;
  int skip_count;
  int pass_count;
  std::string error_messages;
  std::string test_name;

 protected:
  bool Count(bool result);
  bool CountSkip();

  bool VerifySame(const char *exp1, const char *exp2,
                  const Variant& v1, const Variant& v2);
  bool VerifyClose(const char *exp1, const char *exp2,
                   double v1, double v2);
  bool array_value_exists(const Variant& var, const Variant& value);

  template<class T>
  bool runTestImpl(T test, const std::string& which, const std::string& name) {
    bool ret = true;
    auto pass = [&] { printf("%s passed\n", name.c_str()); };
    auto fail = [&] { printf("%s failed\n", name.c_str()); ret = false; };

    if (which.empty() || which == name) {
      SCOPE_EXIT { fflush(stdout); fflush(stderr); };
      test_name = name;
      try {
        if (test()) {
          if (!Test::s_quiet) {
            pass();
          }
        } else {
          fail();
        }
      } catch (const std::exception& e) {
        fprintf(stderr, "%s threw %s: '%s'\n",
                name.c_str(), typeid(e).name(), e.what());
        fail();
      } catch (...) {
        fprintf(stderr, "%s threw unknown object\n", name.c_str());
        fail();
      }
    }
    return ret;
  }
};

template <bool value>
struct WithOption {
  explicit WithOption(bool& option) :
    m_option(&option), m_save(option) {
    option = value;
  }
  ~WithOption() { *m_option = m_save; }
private:
  bool *m_option;
  const bool m_save;
};

typedef WithOption<true>  WithOpt;
typedef WithOption<false> WithNoOpt;

///////////////////////////////////////////////////////////////////////////////

namespace test_detail {

inline bool toBoolean(bool    v) { return v;}
inline bool toBoolean(char    v) { return v;}
inline bool toBoolean(short   v) { return v;}
inline bool toBoolean(int     v) { return v;}
inline bool toBoolean(int64_t v) { return v;}
inline bool toBoolean(double  v) { return v;}
inline bool toBoolean(const char* v) = delete;
inline bool toBoolean(const StringData *v) {
  return v ? v->toBoolean() : false;
}
inline bool toBoolean(const String& v) { return toBoolean(v.get());}
inline bool toBoolean(const ArrayData *v) {
  return v && !v->empty();
}
inline bool toBoolean(const Array& v) { return toBoolean(v.get());}
inline bool toBoolean(const ObjectData *v) {
  return v ? v->toBoolean() : false;
}
inline bool toBoolean(const Object& v) { return toBoolean(v.get());}
inline bool toBoolean(const Variant& v) { return v.toBoolean();}

}

///////////////////////////////////////////////////////////////////////////////
// macros

#define RUN_TEST(test) do {                                             \
    if (!runTestImpl([=] { return test(); }, which, #test)) ret = false; \
  } while(false)

#define LOG_TEST_ERROR(...) do {                                        \
    std::string msg;                                                    \
    string_printf(msg, __VA_ARGS__);                                    \
    printf("%s\n", msg.c_str());                                        \
    error_messages += "\n\n" + msg;                                     \
  } while (false)

#define SKIP(reason)                                                    \
  LOG_TEST_ERROR("%s skipped [%s]", __FUNCTION__, #reason);             \
  return CountSkip();                                                   \

#define VERIFY(exp)                                                     \
  if (!test_detail::toBoolean(exp)) {                                   \
    LOG_TEST_ERROR("%s:%d: [%s] is false", __FILE__, __LINE__, #exp);   \
    return Count(false);                                                \
  }                                                                     \

#define VS(e1, e2)                                                      \
  if (!VerifySame(#e1, #e2, e1, e2)) {                                  \
    LOG_TEST_ERROR("%s:%d: VerifySame failed.", __FILE__, __LINE__);    \
    return Count(false);                                                \
  }                                                                     \

#define VC(e1, e2)                                                      \
  if (!VerifyClose(#e1, #e2, e1, e2)) {                                 \
    LOG_TEST_ERROR("%s:%d: VerifyClose failed.", __FILE__, __LINE__);   \
    return Count(false);                                                \
  }                                                                     \

///////////////////////////////////////////////////////////////////////////////

#endif // incl_HPHP_TEST_BASE_H_
