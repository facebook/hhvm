#include "hphp/runtime/base/smart-ptr.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/resource-data.h"

namespace HPHP {

const char* getClassNameCstr(const ResourceData* p) {
  return p->o_getClassName().c_str();
}

const char* getClassNameCstr(const ObjectData* p) {
  return p->getClassName().c_str();
}

}
