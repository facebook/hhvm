/*
 * From
 *  http://devzone.zend.com/303/
 * "hello" was renamed to "dso_test".
 */

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/type-string.h"

namespace HPHP { namespace {

__thread int64_t tl_counter;
const StaticString s_hello_world("Hello World");

String HHVM_FUNCTION(dso_test_world) {
  return s_hello_world;
}

int64_t HHVM_FUNCTION(dso_test_long) {
  return tl_counter++;
}

double HHVM_FUNCTION(dso_test_double) {
  return 3.1415926535;
}

bool HHVM_FUNCTION(dso_test_bool) {
  return true;
}

void HHVM_FUNCTION(dso_test_null) {}

struct TestExtension : Extension {
  TestExtension(): Extension("dso_test", "1.0.0") {}

  void moduleInit() override {
    HHVM_FE(dso_test_world);
    HHVM_FE(dso_test_long);
    HHVM_FE(dso_test_double);
    HHVM_FE(dso_test_bool);
    HHVM_FE(dso_test_null);
  }

  void requestInit() override {
    tl_counter = 0;
  }
} s_dso_test_extension;

}
HHVM_GET_MODULE(dso_test)
}
