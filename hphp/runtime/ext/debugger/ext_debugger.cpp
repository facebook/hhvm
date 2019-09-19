/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/ext/sockets/ext_sockets.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/ext/vsdebug/ext_vsdebug.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/debugger/debugger_proxy.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/unwind.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
TRACE_SET_MOD(debugger);

using namespace Eval;

struct DebuggerExtension final : Extension {
  DebuggerExtension() : Extension("debugger", NO_EXTENSION_VERSION_YET) {}
  void moduleInit() override {
    HHVM_NAMED_FE(__SystemLib\\debugger_get_info, HHVM_FN(debugger_get_info));
    HHVM_FE(hphpd_auth_token);
    HHVM_FE(hphp_debug_session_auth);
    HHVM_FE(hphpd_break);
    HHVM_FE(hphp_debugger_attached);
    HHVM_FE(hphp_debug_break);
    HHVM_FE(hphp_debugger_set_option);
    HHVM_FE(hphp_debugger_get_option);
    loadSystemlib();
  }
} s_debugger_extension;

///////////////////////////////////////////////////////////////////////////////

String HHVM_FUNCTION(hphpd_auth_token) {
  TRACE(5, "in f_hphpd_auth_token()\n");
  if (auto proxy = Debugger::GetProxy()) {
    return String(proxy->requestAuthToken());
  }

  return String();
}

String HHVM_FUNCTION(hphp_debug_session_auth) {
  TRACE(5, "in f_hphp_debug_session_auth()\n");
  if (auto proxy = Debugger::GetProxy()) {
    return String(proxy->requestSessionAuth());
  } else {
    auto debugger = HPHP::VSDEBUG::VSDebugExtension::getDebugger();
    if (debugger != nullptr) {
      return String(debugger->getDebuggerSessionAuth());
    }
  }

  return String();
}

void HHVM_FUNCTION(hphpd_break, bool condition /* = true */) {
  TRACE(5, "in f_hphpd_break()\n");
  f_hphp_debug_break(condition);
  TRACE(5, "out f_hphpd_break()\n");
}

// Hard breakpoint for the VSDebug extension debugger.
bool HHVM_FUNCTION(hphp_debug_break, bool condition /* = true */) {
  TRACE(5, "in f_hphp_debug_break()\n");
  if (!condition || g_context->m_dbgNoBreak) {
    TRACE(5, "bail !%d || !%d || %d\n", RuntimeOption::EnableHphpdDebugger,
          condition, g_context->m_dbgNoBreak);
    return false;
  }

  // Try breaking into the VS Debug Extenstion, if available.
  auto debugger = HPHP::VSDEBUG::VSDebugExtension::getDebugger();
  if (debugger != nullptr) {
    if (debugger->onHardBreak()) {
      return true;
    }
  }

  // Try breaking into hphpd, if attached.
  if (RuntimeOption::EnableHphpdDebugger) {
    VMRegAnchor _;
    Debugger::InterruptVMHook(HardBreakPoint);
    if (RuntimeOption::EvalJit && DEBUGGER_FORCE_INTR &&
        !RuntimeOption::ForceDebuggerBpToInterp) {
      TRACE(5, "switch mode\n");
      throw VMSwitchModeBuiltin();
    }
    return true;
  }

  TRACE(5, "out f_hphp_debug_break()\n");
  return false;
}

// Quickly determine if a debugger is attached to the current thread.
bool HHVM_FUNCTION(hphp_debugger_attached) {
  if (RuntimeOption::EnableHphpdDebugger && (Debugger::GetProxy() != nullptr)) {
    return true;
  }

  auto debugger = HPHP::VSDEBUG::VSDebugExtension::getDebugger();
  return (debugger != nullptr && debugger->clientConnected());
}

bool HHVM_FUNCTION(hphp_debugger_set_option, const String& option, bool value) {
  auto debugger = HPHP::VSDEBUG::VSDebugExtension::getDebugger();
  if (!debugger) {
    raise_error("hphp_debugger_set_option: no debugger extension is enabled");
  } else if (!debugger->clientConnected()) {
    raise_error("hphp_debugger_set_option: no debugger client is attached");
  } else {
    debugger->setDebuggerOption(option, value);
    return value;
  }
}

bool HHVM_FUNCTION(hphp_debugger_get_option, const String& option) {
  auto debugger = HPHP::VSDEBUG::VSDebugExtension::getDebugger();
  if (!debugger) {
    raise_error("hphp_debugger_get_option: no debugger extension is enabled");
  } else if (!debugger->clientConnected()) {
    raise_error("hphp_debugger_get_option: no debugger client is attached");
  } else {
    return debugger->getDebuggerOption(option);
  }
}

const StaticString
  s_clientIP("clientIP"),
  s_clientPort("clientPort");

Array HHVM_FUNCTION(debugger_get_info) {
  Array ret(Array::Create());
  if (!RuntimeOption::EnableHphpdDebugger) return ret;
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
