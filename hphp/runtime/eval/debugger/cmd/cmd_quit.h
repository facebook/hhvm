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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_QUIT_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_QUIT_H_

#include "hphp/runtime/eval/debugger/debugger_command.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdQuit);
class CmdQuit : public DebuggerCommand {
public:
  CmdQuit() : DebuggerCommand(KindOfQuit) {}

  // The text to display when the debugger client processes "help quit".
  virtual void help(DebuggerClient &client);

protected:
  // Carries out the Quit command by informing the server the client
  // is going away and then getting the client to quit.
  virtual void onClientImpl(DebuggerClient &client);

};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_QUIT_H_
