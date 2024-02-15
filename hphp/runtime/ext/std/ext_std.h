#ifndef HPHP_EXT_STD_H
#define HPHP_EXT_STD_H

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

struct StandardExtension final : Extension {
  StandardExtension() : Extension("standard", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}

  void moduleRegisterNative() override {
    registerNativeStandard();
    registerNativeErrorFunc();
    registerNativeClassobj();
    registerNativeNetwork();
    registerNativeOptions();
    registerNativeGc();
    registerNativeOutput();
    registerNativeString();
    registerNativeVariable();
    registerNativeFunction();
    registerNativeMisc();
    registerNativeFile();
    registerNativeMath();
    registerNativeProcess();
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
  void registerNativeStandard();
  void registerNativeErrorFunc();
  void registerNativeClassobj();
  void registerNativeNetwork();
  void registerNativeOptions();
  void registerNativeGc();
  void registerNativeOutput();
  void registerNativeString();
  void registerNativeVariable();
  void registerNativeFunction();
  void registerNativeMisc();
  void registerNativeFile();
  void registerNativeMath();
  void registerNativeProcess();

  void threadInitMisc();

  void requestInitMath();
};

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // HPHP_EXT_STD_H
