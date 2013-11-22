/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdNext);
class CmdNext : public CmdFlowControl {
public:
  CmdNext() :
      CmdFlowControl(KindOfNext)
      , m_stepContTag(nullptr)
      , m_skippingAsyncESuspend(false)
    {}

  virtual void help(DebuggerClient& client);
  virtual void onSetup(DebuggerProxy& proxy, CmdInterrupt& interrupt);
  virtual void onBeginInterrupt(DebuggerProxy& proxy, CmdInterrupt& interrupt);

private:
  void stepCurrentLine(CmdInterrupt& interrupt, ActRec* fp, PC pc);
  void stepAfterAsyncESuspend();
  bool hasStepCont();
  bool atStepContOffset(Unit* unit, Offset o);
  void setupStepCont(ActRec* fp, PC pc);
  void cleanupStepCont();
  void* getContinuationTag(ActRec* fp);

  StepDestination m_stepCont;
  void* m_stepContTag; // Unique identifier for the continuation we're stepping
  bool m_skippingAsyncESuspend;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_NEXT_H_
