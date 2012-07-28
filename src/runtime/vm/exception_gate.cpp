/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/execution_context.h>

namespace HPHP {
namespace VM {

/*
 * An exception gate backstops entries into C++ that could throw php-level
 * exceptions. It wraps semi-arbitrary C++ with a try/catch block that
 * bubbles exceptions into the VM as appropriate.
 *
 * The macros here are heinous; they open scopes and stuff. Do not play on
 * or around. XXX Once C++11 hits us, and we believe in our compiler's
 * ability to inline some very simple closures, redo this.
 */
static void pushFault(Fault::FaultType t, Exception* e, Object* o = NULL) {
  VMExecutionContext* ec = g_vmContext;
  Fault fault;
  fault.m_faultType = t;
  if (t == Fault::KindOfUserException) {
    // User object.
    ASSERT(o);
    fault.m_userException = o->get();
    fault.m_userException->incRefCount();
  } else {
    fault.m_cppException = e;
  }
  ec->m_faults.push_back(fault);
}

int exception_gate_handle() {
  int longJmpType;
  try {
    throw;
  } catch (Object& e) {
    pushFault(HPHP::VM::Fault::KindOfUserException, NULL, &e);
    longJmpType = g_vmContext->hhvmPrepareThrow();
  } catch (VMSwitchModeException &e) {
    longJmpType = g_vmContext->switchMode(e.unwindBuiltin());
  } catch (Exception &e) {
    pushFault(HPHP::VM::Fault::KindOfCPPException, e.clone());
    longJmpType = g_vmContext->hhvmPrepareThrow();
  } catch (...) {
    pushFault(HPHP::VM::Fault::KindOfCPPException,
              new Exception("unknown exception"));
    longJmpType = g_vmContext->hhvmPrepareThrow();
  }
  return longJmpType;
}

} } // HPHP::VM
