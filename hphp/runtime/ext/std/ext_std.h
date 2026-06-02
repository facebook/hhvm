#pragma once

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

struct StandardExtension final : Extension {
  StandardExtension() : Extension("standard", NO_EXTENSION_VERSION_YET, "hphp_hphpi") {}

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
      "ext_std_classobj.php",
      "ext_std_errorfunc.php",
      "ext_std_file.php",
      "ext_std_function.php",
      "ext_std_gc.php",
      "ext_std_math.php",
      "ext_std_misc.php",
      "ext_std_network.php",
      "ext_std_options.php",
      "ext_std_output.php",
      "ext_std_process.php",
      "ext_std_string.php",
      "ext_std_variable.php",
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
