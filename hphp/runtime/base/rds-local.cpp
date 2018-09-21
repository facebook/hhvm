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
namespace RDSLocalDetail {

RDSLocalNode* head = nullptr;

// Hot rds local storage
//
// TODO(T32634221): Add copying logic.  During request swap.
// This rds local holds the saved hot rds locals.  Hot rds locals
// are stored directly in thread local storage, and are copied in and out of
// this rds local as the request becomes active, or inactive.

static RDS_LOCAL(HotRDSLocals, rl_hotBackingStore);
__thread HotRDSLocals rl_hotSection;
size_t s_rds_local_usedbytes = 0;

void initializeRequestEventHandler(RequestEventHandler* h) {
  h->setInited(true);
  // This registration makes sure obj->requestShutdown() will be called. Do
  // it before calling requestInit() so that obj is reachable to the GC no
  // matter what the callback does.
  auto index = g_context->registerRequestEventHandler(h);
  h->requestInit();

  SCOPE_FAIL {
    h->setInited(false);
    g_context->unregisterRequestEventHandler(h, index);
  };
}

}
}
