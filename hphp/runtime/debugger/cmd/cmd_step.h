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

#include "hphp/runtime/debugger/cmd/cmd_flow_control.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

struct CmdStep : CmdFlowControl {
  CmdStep() : CmdFlowControl(KindOfStep) {}

  void help(DebuggerClient&) override;
  void onSetup(DebuggerProxy&, CmdInterrupt&) override;
  void onBeginInterrupt(DebuggerProxy&, CmdInterrupt&) override;
};

///////////////////////////////////////////////////////////////////////////////
}}

