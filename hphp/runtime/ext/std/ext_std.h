#ifndef HPHP_EXT_STD_H
#define HPHP_EXT_STD_H

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

class StandardExtension : public Extension {
 public:
  StandardExtension() : Extension("standard") {}

  void moduleInit() override {
    initErrorFunc();
    initClassobj();
    initNetwork();
    initOptions();
    initOutput();
    initString();
    initVariable();
    initFunction();
    initMisc();
    initStreamUserFilters();
  }

  void threadInit() {
    threadInitMisc();
  }

 private:
  void initErrorFunc();
  void initClassobj();
  void initNetwork();
  void initOptions();
  void initOutput();
  void initString();
  void initVariable();
  void initFunction();
  void initMisc();
  void initStreamUserFilters();

  void threadInitMisc();
};

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // HPHP_EXT_STD_H
