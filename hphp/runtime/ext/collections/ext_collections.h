#ifndef incl_HPHP_EXT_COLLECTIONS_H
#define incl_HPHP_EXT_COLLECTIONS_H

#include "hphp/runtime/ext/extension.h"

namespace HPHP { namespace collections {
/////////////////////////////////////////////////////////////////////////////

class CollectionsExtension : public Extension {
 public:
  CollectionsExtension(): Extension("collections") {}

  void moduleInit() override {
    initPair();
    initVector();
    initMap();
    initSet();
  }

  virtual const SystemlibSet getSystemlibSources() const override {
    return SystemlibSet({
      "collections-map", "collections-pair",
      "collections-set", "collections-vector"
    });
  }

 private:
  void initPair();
  void initVector();
  void initMap();
  void initSet();
};

/////////////////////////////////////////////////////////////////////////////
}}
#endif
