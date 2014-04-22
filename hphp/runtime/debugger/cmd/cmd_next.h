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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_NEXT_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_NEXT_H_

#include "hphp/runtime/debugger/cmd/cmd_flow_control.h"
#include "hphp/runtime/vm/bytecode.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class CmdNext : public CmdFlowControl {
public:
  CmdNext() :
      CmdFlowControl(KindOfNext)
      , m_stepResumableId(nullptr)
      , m_skippingAwait(false)
    {}

  virtual void help(DebuggerClient& client);
  virtual void onSetup(DebuggerProxy& proxy, CmdInterrupt& interrupt);
  virtual void onBeginInterrupt(DebuggerProxy& proxy, CmdInterrupt& interrupt);

private:
  void stepCurrentLine(CmdInterrupt& interrupt, ActRec* fp, PC pc);
  void stepAfterAwait();
  bool hasStepResumable();
  bool atStepResumableOffset(Unit* unit, Offset o);
  void setupStepSuspend(ActRec* fp, PC pc);
  void cleanupStepResumable();
  void* getResumableId(ActRec* fp);

  StepDestination m_stepResumable;
  ActRec* m_stepResumableId;  // Unique id for the resumable we're stepping
  bool m_skippingAwait;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_NEXT_H_
