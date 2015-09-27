#ifndef HPHP_EXT_STD_H
#define HPHP_EXT_STD_H

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

class StandardExtension final : public Extension {
 public:
  StandardExtension() : Extension("standard") {}

  void moduleInit() override {
    initStandard();
    initErrorFunc();
    initClassobj();
    initNetwork();
    initOptions();
    initGc();
    initOutput();
    initString();
    initVariable();
    initFunction();
    initMisc();
    initStreamUserFilters();
    initFile();
    initIntrinsics();
    initMath();
  }

  virtual const SystemlibSet getSystemlibSources() const override {
    return SystemlibSet({
      "std_classobj", "std_errorfunc", "std_file", "std_function",
      "std_gc", "std_intrinsics", "std_math", "std_misc",
      "std_network", "std_options", "std_output", "std_process",
      "std_string", "std_variable", "stream-user-filters"
    });
  }

  void threadInit() override {
    threadInitMisc();
  }

  void requestInit() override;
 private:
  void initStandard();
  void initErrorFunc();
  void initClassobj();
  void initNetwork();
  void initOptions();
  void initGc();
  void initOutput();
  void initString();
  void initVariable();
  void initFunction();
  void initMisc();
  void initStreamUserFilters();
  void initFile();
  void initIntrinsics();
  void initMath();

  void threadInitMisc();
};

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // HPHP_EXT_STD_H
