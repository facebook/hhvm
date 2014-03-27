/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/debugger/ext_debugger.h"
#include "hphp/runtime/ext/ext_socket.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/debugger/debugger_proxy.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/unwind.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
TRACE_SET_MOD(debugger);

using namespace Eval;
using HPHP::JIT::CallerFrame;

class DebuggerExtension : public Extension {
 public:
  DebuggerExtension() : Extension("debugger", NO_EXTENSION_VERSION_YET) {}
  virtual void moduleInit() override {
    HHVM_NAMED_FE(__SystemLib\\debugger_get_info, HHVM_FN(debugger_get_info));
    loadSystemlib();
  }
} s_debugger_extension;

///////////////////////////////////////////////////////////////////////////////

void f_hphpd_break(bool condition /* = true */) {
  TRACE(5, "in f_hphpd_break()\n");
  if (!RuntimeOption::EnableDebugger || !condition ||
      g_context->m_dbgNoBreak) {
    TRACE(5, "bail !%d || !%d || %d\n", RuntimeOption::EnableDebugger,
          condition, g_context->m_dbgNoBreak);
    return;
  }
  CallerFrame cf;
  Debugger::InterruptVMHook(HardBreakPoint);
  if (RuntimeOption::EvalJit && DEBUGGER_FORCE_INTR) {
    TRACE(5, "switch mode\n");
    throw VMSwitchModeBuiltin();
  }
  TRACE(5, "out f_hphpd_break()\n");
}

// Quickly determine if a debugger is attached to the current thread.
bool f_hphp_debugger_attached() {
  return (RuntimeOption::EnableDebugger && (Debugger::GetProxy() != nullptr));
}

const StaticString
  s_clientIP("clientIP"),
  s_clientPort("clientPort");

Array HHVM_FUNCTION(debugger_get_info) {
  Array ret(Array::Create());
  if (!RuntimeOption::EnableDebugger) return ret;
  DebuggerProxyPtr proxy = Debugger::GetProxy();
  if (!proxy) return ret;
  Variant address;
  Variant port;
  if (proxy->getClientConnectionInfo(ref(address), ref(port))) {
    ret.set(s_clientIP, address);
    ret.set(s_clientPort, port);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
