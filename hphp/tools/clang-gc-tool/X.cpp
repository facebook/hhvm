#include "X.h"

namespace ns {

struct privateParent {
  int x;
  HPHP::Variant var;
};

class privateX : public HPHP::ResourceData, public privateParent {
  void method() const { }
  char* x;
};

}
