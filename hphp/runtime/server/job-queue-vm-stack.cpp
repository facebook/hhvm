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

#include "hphp/runtime/server/job-queue-vm-stack.h"

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/server/xbox-request-handler.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/ext/json/JSON_parser.h"

namespace HPHP {

void JobQueueDropVMStack::dropCache() {
  tl_heap->flush();

  flush_evaluation_stack();

  rds::flush();
  json_parser_flush_caches();

  always_assert(tl_heap->empty());
}

}
