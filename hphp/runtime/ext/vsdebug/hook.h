/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_VSDEBUG_HOOK_H_
#define incl_HPHP_VSDEBUG_HOOK_H_

#include "hphp/runtime/vm/debugger-hook.h"

namespace HPHP {
namespace VSDEBUG {

struct Debugger;
struct DebuggerRequestInfo;

struct VSDebugHook final : DebuggerHook {
  void onRequestInit() override;

  void onOpcode(PC pc) override;

  void onFuncEntryBreak(const Func* func) override;
  void onFuncExitBreak(const Func* func) override;
  void onLineBreak(const Unit* unit, int line) override;

  void onExceptionThrown(ObjectData* exception) override;
  void onError(
    const ExtendedException& extendedException,
    int errnum,
    const std::string& message) override;

  void onStepInBreak(const Unit* unit, int line) override;
  void onStepOutBreak(const Unit* unit, int line) override;
  void onNextBreak(const Unit* unit, int line) override;

  void onFileLoad(Unit* efile) override;
  void onDefClass(const Class* cls) override;
  void onDefFunc(const Func* func) override;
  void onRegisterFuncIntercept(const String& name) override;

  static DebuggerHook* GetInstance() {
    static VSDebugHook* hook = new VSDebugHook();
    return static_cast<DebuggerHook*>(hook);
  }

  static void tryEnterDebugger(
    Debugger* debugger,
    DebuggerRequestInfo* requestInfo,
    bool breakNoStepOnly
  );

private:
  explicit VSDebugHook() {}
  ~VSDebugHook() {}

  static const StaticString s_memoryLimit;
  static const StaticString s_getMsg;
};

}
}

#endif // incl_HPHP_VSDEBUG_HOOK_H_
