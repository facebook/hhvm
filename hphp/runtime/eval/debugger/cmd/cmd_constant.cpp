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

#include "hphp/runtime/eval/debugger/cmd/cmd_constant.h"
#include "hphp/runtime/base/class_info.h"
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

bool CmdConstant::help(DebuggerClient *client) {
  client->helpTitle("Constant Command");
  client->helpCmds(
    "[k]onstant",           "lists all constants",
    "[k]onstant {text}",    "full-text search constants",
    nullptr
  );
  client->helpBody(
    "This will print names and values of all constants, if {text} is not "
    "speified. Otherwise, it will print names and values of all constants "
    "that contain the text in their names or values. The search is case-"
    "insensitive and string-based."
  );
  return true;
}

bool CmdConstant::onClientImpl(DebuggerClient *client) {
  if (DebuggerCommand::onClientImpl(client)) return true;

  String text;
  if (client->argCount() == 1) {
    text = client->argValue(1);
  } else if (client->argCount() != 0) {
    return help(client);
  }

  CmdConstantPtr cmd = client->xend<CmdConstant>(this);
  if (cmd->m_constants.empty()) {
    client->info("(no constant was defined)");
  } else {
    int i = 0;
    bool found = false;
    m_constants = cmd->m_constants;
    f_ksort(ref(m_constants));
    for (ArrayIter iter(m_constants); iter; ++iter) {
      String name = iter.first().toString();
      String value = DebuggerClient::FormatVariable(iter.second(), 200);
      if (!text.empty()) {
        String fullvalue = DebuggerClient::FormatVariable(iter.second(), -1);
        if (name.find(text, 0, false) >= 0 ||
            fullvalue.find(text, 0, false) >= 0) {
          client->print("%s = %s", name.data(), value.data());
          found = true;
        }
      } else {
        client->print("%s = %s", name.data(), value.data());
        ++i;
        if (!client->isApiMode() &&
            i % DebuggerClient::ScrollBlockSize == 0 &&
            client->ask("There are %d more constants. Continue? [Y/n]",
                        m_constants.size() - i) == 'n') {
          break;
        }
      }
    }

    if (!text.empty() && !found) {
      client->info("(unable to find specified text in any constants)");
    }
  }

  return true;
}

void CmdConstant::setClientOutput(DebuggerClient *client) {
  client->setOutputType(DebuggerClient::OTValues);
  Array values;
  for (ArrayIter iter(m_constants); iter; ++iter) {
    String name = iter.first().toString();
    if (client->getDebuggerClientApiModeSerialize()) {
      values.set(name,
                 DebuggerClient::FormatVariable(iter.second(), 200));
    } else {
      values.set(name, iter.second());
    }
  }
  client->setOTValues(values);
}

bool CmdConstant::onServer(DebuggerProxy *proxy) {
  try {
    m_constants = ClassInfo::GetConstants();
  } catch (...) {}
  return proxy->sendToClient(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
