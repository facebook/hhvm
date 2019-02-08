/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/rds-local.h"

#include "hphp/runtime/base/execution-context.h"

namespace HPHP {
namespace rds {
namespace local {
namespace detail {

RDSLocalNode* head = nullptr;

// Hot rds local storage
//
// TODO(T32634221): Add copying logic.  During request swap.
// This rds local holds the saved hot rds locals.  Hot rds locals
// are stored directly in thread local storage, and are copied in and out of
// this rds local as the request becomes active, or inactive.

static RDS_LOCAL(HotRDSLocals, rl_hotBackingStore);
alignas(64) __thread HotRDSLocals rl_hotSection = {};
uint32_t s_usedbytes = 0;

Handle RDSLocalNode::s_RDSLocalsBase;

void initializeRequestEventHandler(RequestEventHandler* h) {
  h->setInited(true);
  // This registration makes sure obj->requestShutdown() will be called. Do
  // it before calling requestInit() so that obj is reachable to the GC no
  // matter what the callback does.
  auto index = g_context->registerRequestEventHandler(h);
  SCOPE_FAIL {
    h->setInited(false);
    g_context->unregisterRequestEventHandler(h, index);
  };

  h->requestInit();
}

///////////////////////////////////////////////////////////////////////////////
}

void RDSInit() {
  assertx(!isHandleBound(detail::RDSLocalNode::s_RDSLocalsBase));
  detail::RDSLocalNode::s_RDSLocalsBase =
    rds::detail::allocUnlocked(Mode::Local, std::max(detail::s_usedbytes, 16U),
                               16U, type_scan::kIndexUnknown);
}

void init() {
  // We may have already been initialized in the case of a thread creating an
  // RDS segment after we already decided to malloc some memory for RDS locals.
  // This should not happen on request threads or RDS locals might not be
  // accessible from the JIT.
  if (detail::rl_hotSection.rdslocal_base != nullptr) return;
  if (tl_base) {
    detail::rl_hotSection.rdslocal_base =
      handleToPtr<void, Mode::Local>(detail::RDSLocalNode::s_RDSLocalsBase);
  } else {
    detail::rl_hotSection.rdslocal_base = malloc(detail::s_usedbytes);
  }
  always_assert(detail::rl_hotSection.rdslocal_base);
  always_assert((uintptr_t)detail::rl_hotSection.rdslocal_base % 16 == 0);
  detail::iterate([](detail::RDSLocalNode* p) { p->init(); });
}

void fini(bool inrds) {
  // This may be called twice on threads that create a malloced rdslocal area,
  // and then initialize a full RDS segment.  As the RDS segment is detroyed
  // Fini is called, and it is called again from the context that created the
  // malloced area.
  if (!detail::rl_hotSection.rdslocal_base) return;

  if (inrds != (tl_base &&
                std::less_equal<void>()(localSection().cbegin(),
                                        detail::rl_hotSection.rdslocal_base)
                && std::less_equal<void>()(
                  (const char*)detail::rl_hotSection.rdslocal_base
                  + detail::s_usedbytes, localSection().cend()))) {
    // There will be another call to deallocate the rds local section.
    return;
  }
  detail::iterate([](detail::RDSLocalNode* p) { p->fini(); });
  if (!inrds) {
    free(detail::rl_hotSection.rdslocal_base);
  }
  detail::rl_hotSection.rdslocal_base = nullptr;
}

}}}
