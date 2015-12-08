#ifndef HPHP_EXT_STD_H
#define HPHP_EXT_STD_H

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

class StandardExtension final : public Extension {
 public:
  StandardExtension() : Extension("standard") {}

  void moduleLoad(const IniSetting::Map& ini, Hdf config) override {
    // Closure must be hoisted before anything which extends from it.
    // So we place it in the global systemlib and bind its dependencies early.
    loadClosure();
  }

  void moduleInit() override {
    initClosure();
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
    initProcess();
  }

  void threadInit() override {
    threadInitMisc();
  }

  void requestInit() override {
    requestInitMath();
    requestInitOptions();
  }
 private:
  void loadClosure();

  void initStandard();
  void initErrorFunc();
  void initClassobj();
  void initClosure();
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
  void initProcess();

  void threadInitMisc();

  void requestInitMath();
  void requestInitOptions();
};

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // HPHP_EXT_STD_H
