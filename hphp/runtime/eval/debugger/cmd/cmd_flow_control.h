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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_FLOW_CONTROL_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_FLOW_CONTROL_H_

#include <runtime/eval/debugger/debugger_command.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdFlowControl);
class CmdFlowControl : public DebuggerCommand {
public:
  explicit CmdFlowControl(Type type)
      : DebuggerCommand(type), m_count(1), m_stackDepth(0), m_vmDepth(0) { }

  int decCount() { assert(m_count > 0); return --m_count;}
  int getCount() const { assert(m_count > 0); return m_count;}
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
  int16_t m_count;
  std::string m_loc; // last break's source location
  bool m_smallStep;

  int m_stackDepth;
  int m_vmDepth;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_FLOW_CONTROL_H_
