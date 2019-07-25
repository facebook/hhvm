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

#include "hphp/util/rds-local.h"

#include "hphp/util/alloc.h"

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

uint32_t RDSLocalNode::s_RDSLocalsBase = 0;

Configuration g_config;
///////////////////////////////////////////////////////////////////////////////
}

void RDSInit() {
  assertx(detail::g_config.rdsInitFunc);
  detail::RDSLocalNode::s_RDSLocalsBase =
    detail::g_config.rdsInitFunc(detail::s_usedbytes);
}

void init() {
  // We may have already been initialized in the case of a thread creating an
  // RDS segment after we already decided to malloc some memory for RDS locals.
  // This should not happen on request threads or RDS locals might not be
  // accessible from the JIT.
  if (detail::rl_hotSection.rdslocal_base != nullptr) return;

  assertx(detail::g_config.initFunc);
  detail::rl_hotSection.rdslocal_base =
    detail::g_config.initFunc(detail::s_usedbytes,
                              detail::RDSLocalNode::s_RDSLocalsBase);

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

  if (detail::g_config.inRdsFunc &&
      inrds != detail::g_config.inRdsFunc(detail::rl_hotSection.rdslocal_base,
                                  detail::s_usedbytes)) {
    // There will be another call to deallocate the rds local section.
    return;
  }
  detail::iterate([](detail::RDSLocalNode* p) { p->fini(); });
  if (!inrds) {
    assertx(detail::g_config.finiFunc);
    detail::g_config.finiFunc(detail::rl_hotSection.rdslocal_base);
  }
  detail::rl_hotSection.rdslocal_base = nullptr;
}

}}}
