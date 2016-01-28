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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_INTERRUPT_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_INTERRUPT_H_

#include <vector>

#include "hphp/runtime/debugger/break_point.h"
#include "hphp/runtime/debugger/debugger_command.h"
#include "hphp/util/process.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

struct CmdInterrupt : DebuggerCommand {
  CmdInterrupt() : DebuggerCommand(KindOfInterrupt) {}

  CmdInterrupt(InterruptType interrupt, const char *program,
               InterruptSite *site, const char *error)
      : DebuggerCommand(KindOfInterrupt)
      , m_interrupt(interrupt)
      , m_program(program ? program : "")
      , m_site(site)
  {
    m_threadId = (int64_t)Process::GetThreadId();
    if (error) m_errorMsg = error;
  }

  int64_t getThreadId() const {
    return m_threadId;
  }

  InterruptType getInterruptType() const {
    return (InterruptType)m_interrupt;
  }

  std::string desc() const;

  const std::string& error() const {
    return m_errorMsg;
  }

  bool onServer(DebuggerProxy&) override;
  void onClient(DebuggerClient&) override;

  bool shouldBreak(DebuggerProxy &proxy,
                   const std::vector<BreakPointInfoPtr> &bps,
                   int stackDepth);
  std::string getFileLine() const;

  InterruptSite* getSite() {
    return m_site;
  }

protected:
  void sendImpl(DebuggerThriftBuffer&) override;
  void recvImpl(DebuggerThriftBuffer&) override;

private:
  int16_t m_interrupt{-1};
  std::string m_program;   // informational only
  std::string m_errorMsg;  // informational only
  int64_t m_threadId{0};
  InterruptSite* m_site{nullptr};   // server side
  BreakPointInfoPtr m_bpi; // client side
  std::vector<BreakPointInfoPtr> m_matched;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_INTERRUPT_H_
