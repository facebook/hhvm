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
