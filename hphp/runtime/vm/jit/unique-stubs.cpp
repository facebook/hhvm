/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/unique-stubs.h"

#include "hphp/util/disasm.h"
#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(ustubs);

//////////////////////////////////////////////////////////////////////

TCA UniqueStubs::add(const char* name, TCA start) {
  auto const inAStubs = start > mcg->code.stubs().base();
  UNUSED auto const stop = inAStubs ? mcg->code.stubs().frontier()
    : mcg->code.main().frontier();

  FTRACE(1, "unique stub: {} {} -- {:4} bytes: {}\n",
         inAStubs ? "astubs @ "
         : "  ahot @  ",
         static_cast<void*>(start),
         static_cast<size_t>(stop - start),
         name);

  ONTRACE(2,
          [&]{
            Disasm dasm(Disasm::Options().indent(4));
            std::ostringstream os;
            dasm.disasm(os, start, stop);
            FTRACE(2, "{}\n", os.str());
          }()
         );

  mcg->recordGdbStub(
    mcg->code.blockFor(start),
    start,
    strdup(folly::format("HHVM::{}", name).str().c_str())
  );
  return start;
}

//////////////////////////////////////////////////////////////////////

}}
