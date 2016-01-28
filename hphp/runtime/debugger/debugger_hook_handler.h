/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_EVAL_HOOK_HANDLER_H_
#define incl_EVAL_HOOK_HANDLER_H_

#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/vm/debugger-hook.h"

namespace HPHP { namespace Eval {

/*
 * Called by the proxy whenever its breakpoint list is updated.  Since this
 * intended to be called when user input is received, it is not performance
 * critical.  Also, in typical scenarios, the list is short.
 */
void proxySetBreakPoints(DebuggerProxy* proxy);

/* Debugger hook for hphpd. */
struct HphpdHook : DebuggerHook {
  static DebuggerHook* GetInstance();

  void onOpcode(const unsigned char* pc) override {
    Debugger::InterruptVMHook();
  }

  void onExceptionThrown(ObjectData* exception) override {
    Debugger::InterruptVMHook(ExceptionThrown, Variant{exception});
  }

  void onExceptionHandle() override {
    Debugger::InterruptVMHook(ExceptionHandler);
  }

  void onError(
    const ExtendedException& ee,
    int errnum,
    const std::string& message
  ) override {
    Debugger::InterruptVMHook(ExceptionThrown, String(message));
  }

  void onFileLoad(Unit* unit) override;
  void onDefClass(const Class* cls) override;
  void onDefFunc(const Func* f) override;
private:
  HphpdHook() {}
  ~HphpdHook() override {}
};

}}

#endif
