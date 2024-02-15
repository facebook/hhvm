#ifndef HPHP_EXT_STD_H
#define HPHP_EXT_STD_H

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

struct StandardExtension final : Extension {
  StandardExtension() : Extension("standard", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}

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
    initFile();
    initMath();
    initProcess();
  }

  std::vector<std::string> hackFiles() const override {
    return {
      "std_classobj",
      "std_errorfunc",
      "std_file",
      "std_function",
      "std_gc",
      "std_math",
      "std_misc",
      "std_network",
      "std_options",
      "std_output",
      "std_process",
      "std_string",
      "std_variable",
    };
  }

  void threadInit() override {
    threadInitMisc();
  }

  void requestInit() override {
    requestInitMath();
  }

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
  void initFile();
  void initMath();
  void initProcess();

  void threadInitMisc();

  void requestInitMath();
};

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // HPHP_EXT_STD_H
