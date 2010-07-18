/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/eval/debugger/cmd/cmd_next.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdNext::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
}

void CmdNext::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
}

bool CmdNext::help(DebuggerClient *client) {
  client->error("not implemented yet"); return true;

  client->helpTitle("Next Command");
  client->help("next: ");
  client->helpBody(
    ""
  );
  return true;
}

bool CmdNext::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;

  //TODO

  return help(client);
}

bool CmdNext::onServer(DebuggerProxy *proxy) {
  ASSERT(false); // this command is processed entirely locally
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}}
