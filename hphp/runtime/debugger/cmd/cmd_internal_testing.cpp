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

#include "hphp/runtime/debugger/cmd/cmd_internal_testing.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdInternalTesting::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_arg);
  // Write less data on purpose, to get the server to choke on deserialization
  if ((m_arg != "shortcmd")) {
    thrift.write(m_unused);
  }
}

void CmdInternalTesting::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_arg);
  thrift.read(m_unused);
}

void CmdInternalTesting::help(DebuggerClient &client) {
  TRACE(2, "CmdInternalTesting::help\n");
  client.helpTitle("Internal Testing Command");
  client.helpCmds(
    "badcmdtypesend", "Send a bad command type to the proxy",
    "badcmdtypereceive", "Receive a bad command type from the proxy",
    "shortcmdsend", "Send less data that the proxy expects",
    "shortcmdreceive", "Receive less data than the client expects",
    "segfaultClient", "Segfault on the client",
    "segfaultServer", "Segfault on the server",
    nullptr
  );
  client.helpBody(
    "This command is only for internal testing of the debugger, both client "
    "and server. If you're using this command and you're not trying to test "
    "the debugger, then you're making a really big mistake."
  );
}

void CmdInternalTesting::onClient(DebuggerClient &client) {
  TRACE(2, "CmdInternalTesting::onClient\n");
  if (DebuggerCommand::displayedHelp(client)) return;
  if (client.argCount() == 0) {
    help(client);
    return;
  }

  client.info("Executing internal test...");
  m_arg = client.argValue(1);

  if (client.arg(1, "badcmdtypesend")) {
    // Give the cmd a bad type and send it over. This should cause the proxy to
    // disconnect from us.
    m_type = KindOfInternalTestingBad;
    client.sendToServer(this);
    // Spin here and wait for the client to be marked as stopped
    // before going back to the event loop. This will give the local
    // proxy time to recgonize the bad cmd, terminate, and wait for
    // the client to stop. This will ensure that we always exit on the
    // same path on both proxy and client threads, and remove any
    // spurious output form ths test case.
    while (!client.internalTestingIsClientStopped()) {
      sleep(1);
    }
    throw DebuggerConsoleExitException(); // Expect no response
  } else if (client.arg(1, "badcmdtypereceive")) {
    client.xend<CmdInternalTesting>(this);
    return;
  } else if (client.arg(1, "shortcmdsend")) {
    m_arg = "shortcmd"; // Force send to drop a field.
    client.sendToServer(this);
    // See note above about this wait.
    while (!client.internalTestingIsClientStopped()) {
      sleep(1);
    }
    throw DebuggerConsoleExitException(); // Expect no response
  } else if (client.arg(1, "shortcmdreceive")) {
    client.xend<CmdInternalTesting>(this);
    return;
  } else if (client.arg(1, "segfaultClient")) {
    int *px = nullptr;
    *px = 42;
  } else if (client.arg(1, "segfaultServer")) {
    client.xend<CmdInternalTesting>(this);
    return;
  }

  help(client);
}

bool CmdInternalTesting::onServer(DebuggerProxy &proxy) {
  TRACE(2, "CmdInternalTesting::onServer\n");
  if (m_arg == "badcmdtypereceive") {
    m_type = KindOfInternalTestingBad; // Send back a bad cmd.
  } else if (m_arg == "shortcmdreceive") {
    m_arg = "shortcmd"; // Force send to drop a field
  } else if (m_arg == "segfaultServer") {
    int *px = nullptr;
    *px = 42;
  }
  return proxy.sendToClient(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
