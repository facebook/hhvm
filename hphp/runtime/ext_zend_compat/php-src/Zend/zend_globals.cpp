#include "zend.h"
#include "zend_globals.h"
#include "zend_exceptions.h"

#include "hphp/runtime/base/externals.h"
#include "hphp/util/thread-local.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/request-injection-data.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/request-event-handler.h"

ZEND_API zend_compiler_globals compiler_globals;

namespace HPHP {

struct ZendExecutorGlobals final : RequestEventHandler {
  void requestInit() override {
    // Clear out any exceptions that might be left over from previous
    // requests.
    m_data.exception = nullptr;
    m_data.prev_exception = nullptr;
  }

  void requestShutdown() override {
    if (auto exn = m_data.exception) {
      m_data.exception = nullptr;
      zval_ptr_dtor(&exn);
    }
    if (auto exn = m_data.prev_exception) {
      m_data.prev_exception = nullptr;
      zval_ptr_dtor(&exn);
    }
  }
  void vscan(IMarker& mark) const override {}

  ~ZendExecutorGlobals() {}

  _zend_executor_globals m_data;
};

IMPLEMENT_STATIC_REQUEST_LOCAL(ZendExecutorGlobals, s_zend_executor_globals);

} // namespace HPHP

#define G(TYPE, MEMBER)                                   \
  std::add_lvalue_reference<TYPE>::type EG_ ## MEMBER() { \
    return HPHP::s_zend_executor_globals.get()->m_data.MEMBER;         \
  }
EG_DEFAULT
#undef G

HashTable& EG_regular_list() {
  return *HPHP::s_zend_executor_globals.get()->m_data.regular_list;
}
HashTable& EG_persistent_list() {
  return *HPHP::s_zend_executor_globals.get()->m_data.persistent_list;
}

HashTable& EG_symbol_table() {
  return *HPHP::get_global_variables()->asArrayData();
}

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

ZendWrappedErrorReporting g_zend_wrapped_error_reporting;

void ZendWrappedErrorReporting::operator=(int newLevel) {
  ThreadInfo::s_threadInfo.getNoCheck()
    ->m_reqInjectionData.setErrorReportingLevel(newLevel);
}

ZendWrappedErrorReporting::operator int() const {
  return ThreadInfo::s_threadInfo.getNoCheck()
    ->m_reqInjectionData.getErrorReportingLevel();
}

///////////////////////////////////////////////////////////////////////////////
}
