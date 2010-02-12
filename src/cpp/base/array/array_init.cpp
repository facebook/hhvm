#include <cpp/base/array/array_init.h>
#include <cpp/base/array/zend_array.h>
#include <cpp/base/runtime_option.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// ArrayInit

ArrayInit::ArrayInit(int n) : m_elements(NULL), m_data(NULL) {
  if (RuntimeOption::UseZendArray) {
    m_data = NEW(ZendArray)(n);
  } else {
    // released in create()
    m_elements = new ArrayElementVec(n);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
