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

#ifndef incl_HPHP_TEST_BASE_H_
#define incl_HPHP_TEST_BASE_H_

#include <string>
#include <assert.h>
#include <exception>
#include "hphp/compiler/hphp.h"
#include "hphp/runtime/base/types.h"
#include "hphp/test/ext/test.h"

using namespace HPHP;
///////////////////////////////////////////////////////////////////////////////

class TestBase {
 public:
  TestBase();
  virtual ~TestBase() {}

  virtual bool preTest() { return true; }
  virtual bool RunTests(const std::string &which) = 0;
  virtual bool postTest() { return true; }

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

  static char error_buffer[];

  template<class T>
  bool runTestImpl(T test, const std::string& which, const std::string& name) {
    bool ret = true;
    auto pass = [&] { printf("%s passed\n", name.c_str()); };
    auto fail = [&] { printf("%s failed\n", name.c_str()); ret = false; };

    if (which.empty() || which == name) {
      SCOPE_EXIT { fflush(nullptr); };
      test_name = name;
      try {
        if (preTest() && test() && postTest()) {
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
class WithOption {
public:
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
// macros

#define RUN_TEST(test) do {                                             \
    if (!runTestImpl([=] { return test(); }, which, #test)) ret = false; \
  } while(false)

#define LOG_TEST_ERROR(...)                                             \
  sprintf(TestBase::error_buffer, __VA_ARGS__);                         \
  printf("%s\n", TestBase::error_buffer);                               \
  error_messages += "\n\n";                                             \
  error_messages += TestBase::error_buffer;                             \

#define SKIP(reason)                                                    \
  LOG_TEST_ERROR("%s skipped [%s]", __FUNCTION__, #reason);             \
  return CountSkip();                                                   \

#define VERIFY(exp)                                                     \
  if (!toBoolean(exp)) {                                                \
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
