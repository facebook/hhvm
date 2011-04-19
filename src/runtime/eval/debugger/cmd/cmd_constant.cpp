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

#include <runtime/eval/debugger/cmd/cmd_constant.h>
#include <runtime/base/class_info.h>
#include <runtime/ext/ext_array.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

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
    NULL
  );
  client->helpBody(
    "This will print names and values of all constants, if {text} is not "
    "speified. Otherwise, it will print names and values of all constants "
    "that contain the text in their names or values. The search is case-"
    "insensitive and string-based."
  );
  return true;
}

bool CmdConstant::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;

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
    Variant constants = cmd->m_constants;
    f_ksort(ref(constants));
    for (ArrayIter iter(constants); iter; ++iter) {
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
        if (++i % DebuggerClient::ScrollBlockSize == 0 &&
            client->ask("There are %d more constants. Continue? [Y/n]",
                        constants.toArray().size() - i) == 'n') {
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

bool CmdConstant::onServer(DebuggerProxy *proxy) {
  try {
    m_constants = ClassInfo::GetConstants();
  } catch (...) {}
  return proxy->send(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
