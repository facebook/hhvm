/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/debugger/debugger_command.h"
#include "hphp/runtime/base/req-root.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

struct CmdEval : DebuggerCommand {
  CmdEval() : DebuggerCommand(KindOfEval) {
    m_version = 1;
  }

  void onClient(DebuggerClient&) override;
  bool onServer(DebuggerProxy&) override;

  bool failed() const {
    return m_failed;
  }

protected:
  void sendImpl(DebuggerThriftBuffer&) override;
  void recvImpl(DebuggerThriftBuffer&) override;

private:
  void handleReply(DebuggerClient&);

  req::root<String> m_output;
  int m_frame;
  bool m_bypassAccessCheck{false};
  bool m_failed{false};
};

///////////////////////////////////////////////////////////////////////////////
}}

