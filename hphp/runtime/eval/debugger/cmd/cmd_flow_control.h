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

#ifndef __HPHP_EVAL_DEBUGGER_CMD_FLOW_CONTROL_H__
#define __HPHP_EVAL_DEBUGGER_CMD_FLOW_CONTROL_H__

#include <runtime/eval/debugger/debugger_command.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdFlowControl);
class CmdFlowControl : public DebuggerCommand {
public:
  CmdFlowControl(Type type)
      : DebuggerCommand(type), m_count(1), m_frame(NULL), m_nframe(NULL),
        m_stackDepth(0), m_vmDepth(0) { }

  int decCount() { assert(m_count > 0); return --m_count;}
  int getCount() const { assert(m_count > 0); return m_count;}
  void setFrame(FrameInjection *frame) { m_frame = frame;}
  FrameInjection *getFrame() const { return m_frame;}
  void setNegativeFrame(FrameInjection *frame) { m_nframe = frame;}
  FrameInjection *getNegativeFrame() const { return m_nframe;}
  void setFileLine(const std::string &loc) { m_loc = loc;}
  const std::string &getFileLine() const { return m_loc;}

  virtual void sendImpl(DebuggerThriftBuffer &thrift);
  virtual void recvImpl(DebuggerThriftBuffer &thrift);

  virtual bool onClient(DebuggerClient *client);
  virtual bool onServer(DebuggerProxy *proxy);

  void setStackDepth(int depth) { m_stackDepth = depth; }
  int getStackDepth() const { return m_stackDepth; }
  void setVMDepth(int depth) { m_vmDepth = depth; }
  int getVMDepth() const { return m_vmDepth; }

private:
  int16 m_count;
  FrameInjection *m_frame;  // which frame to break next time
  FrameInjection *m_nframe; // definitely not to break with this frame
  std::string m_loc; // last break's source location
  bool m_smallStep;

  int m_stackDepth;
  int m_vmDepth;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_CMD_FLOW_CONTROL_H__
