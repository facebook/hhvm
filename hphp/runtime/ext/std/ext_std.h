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
      "std_classobj.php",
      "std_errorfunc.php",
      "std_file.php",
      "std_function.php",
      "std_gc.php",
      "std_math.php",
      "std_misc.php",
      "std_network.php",
      "std_options.php",
      "std_output.php",
      "std_process.php",
      "std_string.php",
      "std_variable.php",
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
