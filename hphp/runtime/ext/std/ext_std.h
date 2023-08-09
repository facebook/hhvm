#ifndef HPHP_EXT_STD_H
#define HPHP_EXT_STD_H

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

struct StandardExtension final : Extension {
  StandardExtension() : Extension("standard", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}

  void loadDecls() override {
    loadDeclsFrom("std_classobj");
    loadDeclsFrom("std_errorfunc");
    loadDeclsFrom("std_file");
    loadDeclsFrom("std_function");
    loadDeclsFrom("std_gc");
    loadDeclsFrom("std_intrinsics");
    loadDeclsFrom("std_math");
    loadDeclsFrom("std_misc");
    loadDeclsFrom("std_network");
    loadDeclsFrom("std_options");
    loadDeclsFrom("std_output");
    loadDeclsFrom("std_process");
    loadDeclsFrom("std_string");
    loadDeclsFrom("std_variable");
  }

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
      "std_errorfunc",
      "std_classobj",
      "std_network",
      "std_options",
      "std_gc",
      "std_output",
      "std_string",
      "std_variable",
      "std_function",
      "std_misc",
      "std_file",
      "std_intrinsics",
      "std_math",
      "std_process",
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
