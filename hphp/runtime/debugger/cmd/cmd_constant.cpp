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

#include "hphp/runtime/debugger/cmd/cmd_constant.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/ext/ext_array.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdConstant::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_constants);
}

void CmdConstant::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_constants);
}

void CmdConstant::help(DebuggerClient &client) {
  client.helpTitle("Constant Command");
  client.helpCmds(
    "[k]onstant",           "lists all constants",
    "[k]onstant {text}",    "full-text search constants",
    nullptr
  );
  client.helpBody(
    "This will print names and values of all constants, if {text} is not "
    "specified. Otherwise, it will print names and values of all constants "
    "that contain the text in their names or values. The search is case-"
    "insensitive and string-based."
  );
}

void CmdConstant::onClient(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;

  String text;
  if (client.argCount() == 1) {
    text = client.argValue(1);
  } else if (client.argCount() != 0) {
    help(client);
    return;
  }

  CmdConstantPtr cmd = client.xend<CmdConstant>(this);
  if (cmd->m_constants.empty()) {
    client.info("(no constant was defined)");
  } else {
    int i = 0;
    bool found = false;

    {
      Variant forSort(cmd->m_constants);
      f_ksort(ref(forSort));
      assert(forSort.is(KindOfArray));
      m_constants = forSort.asCell()->m_data.parr;
    }

    for (ArrayIter iter(m_constants); iter; ++iter) {
      String name = iter.first().toString();
      String value = DebuggerClient::FormatVariable(iter.second(), 200);
      if (!text.empty()) {
        String fullvalue = DebuggerClient::FormatVariable(iter.second(), -1);
        if (name.find(text, 0, false) >= 0 ||
            fullvalue.find(text, 0, false) >= 0) {
          client.print("%s = %s", name.data(), value.data());
          found = true;
        }
      } else {
        client.print("%s = %s", name.data(), value.data());
        ++i;
        if (i % DebuggerClient::ScrollBlockSize == 0 &&
            client.ask("There are %zd more constants. Continue? [Y/n]",
                        m_constants.size() - i) == 'n') {
          break;
        }
      }
    }

    if (!text.empty() && !found) {
      client.info("(unable to find specified text in any constants)");
    }
  }
}

bool CmdConstant::onServer(DebuggerProxy &proxy) {
  try {
    m_constants = lookupDefinedConstants();
  } catch (...) {}
  return proxy.sendToClient(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
