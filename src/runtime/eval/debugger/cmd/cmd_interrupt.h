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

#ifndef __HPHP_EVAL_DEBUGGER_CMD_INTERRUPT_H__
#define __HPHP_EVAL_DEBUGGER_CMD_INTERRUPT_H__

#include <runtime/eval/debugger/debugger_command.h>
#include <util/process.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdInterrupt);
class CmdInterrupt : public DebuggerCommand {
public:
  CmdInterrupt()
      : DebuggerCommand(KindOfInterrupt),
        m_interrupt(-1), m_threadId(0), m_site(NULL), m_pendingJump(false) {}

  CmdInterrupt(InterruptType interrupt, const char *program,
               InterruptSite *site, const char *error)
      : DebuggerCommand(KindOfInterrupt),
        m_interrupt(interrupt), m_program(program ? program : ""),
        m_site(site), m_pendingJump(false) {
    m_threadId = Process::GetThreadId();
    if (error) m_errorMsg = error;
  }

  int64 getThreadId() const { return m_threadId;}
  InterruptType getInterruptType() const { return (InterruptType)m_interrupt;}
  std::string desc() const;
  std::string error() const { return m_errorMsg;}

  virtual bool onClient(DebuggerClient *client);
  virtual bool onServer(DebuggerProxy *proxy);

  virtual void sendImpl(DebuggerThriftBuffer &thrift);
  virtual void recvImpl(DebuggerThriftBuffer &thrift);

  bool shouldBreak(const BreakPointInfoPtrVec &bps);
  FrameInjection *getFrame() { return m_site ? m_site->getFrame() : NULL;}
  std::string getFileLine() const;

  InterruptSite *getSite() { return m_site;}

  void setPendingJump() { m_pendingJump = true;}

private:
  int16 m_interrupt;
  std::string m_program;   // informational only
  std::string m_errorMsg;  // informational only
  int64 m_threadId;
  InterruptSite *m_site;   // server side
  BreakPointInfoPtr m_bpi; // client side
  BreakPointInfoPtrVec m_matched;
  bool m_pendingJump;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_CMD_INTERRUPT_H__
