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

#include <runtime/eval/debugger/cmd/cmd_variable.h>
#include <runtime/base/hphp_system.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdVariable::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_frame);
  {
    String sdata;
    DebuggerWireHelpers::WireSerialize(m_variables, sdata);
    thrift.write(sdata);
  }
  thrift.write(m_global);
}

void CmdVariable::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_frame);
  {
    String sdata;
    thrift.read(sdata);
    if (DebuggerWireHelpers::WireUnserialize(sdata, m_variables) !=
        DebuggerWireHelpers::NoError) {
      m_variables = null_array;
      m_wireError = sdata;
    }
  }
  thrift.read(m_global);
}

bool CmdVariable::help(DebuggerClient *client) {
  client->helpTitle("Variable Command");
  client->helpCmds(
    "[v]ariable",           "lists all local variables on stack",
    "[v]ariable {text}",    "full-text search local variables",
    NULL
  );
  client->helpBody(
    "This will print names and values of all variables that are currently "
    "accessible by simple names. Use '[w]here', '[u]p {num}', '[d]own {num}', "
    "'[f]rame {index}' commands to choose a different frame to view variables "
    "at different level of the stack.\n"
    "\n"
    "Specify some free text to print local variables that contain the text "
    "either in their names or values. The search is case-insensitive and "
    "string-based."
  );
  return true;
}

void CmdVariable::PrintVariables(DebuggerClient *client, CArrRef variables,
                                 bool global, CStrRef text) {
  bool system = true;
  int i = 0;
  bool found = false;
  for (ArrayIter iter(variables); iter; ++iter) {
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
      if (global && system) {
        client->print("$%s = %s", name.data(), value.data());
      } else {
        client->output("$%s = %s", name.data(), value.data());
      }

      // we knew this is the last system global
      if (global && name == "http_response_header") {
        client->output("");
        system = false;
      }

      ++i;
      if (!client->isApiMode() &&
          i % DebuggerClient::ScrollBlockSize == 0 &&
          client->ask("There are %d more variables. Continue? [Y/n]",
                      variables.size() - i) == 'n') {
        break;
      }
    }
  }

  if (!text.empty() && !found) {
    client->info("(unable to find specified text in any variables)");
  }
}

bool CmdVariable::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;

  String text;
  if (client->argCount() == 1) {
    text = client->argValue(1);
  } else if (client->argCount() != 0) {
    return help(client);
  }

  m_frame = client->getFrame();
  CmdVariablePtr cmd = client->xend<CmdVariable>(this);
  if (cmd->m_variables.empty()) {
    client->info("(no variable was defined)");
  } else {
    m_variables = cmd->m_variables;
    PrintVariables(client, cmd->m_variables, cmd->m_global, text);
  }

  return true;
}

void CmdVariable::setClientOutput(DebuggerClient *client) {
  client->setOutputType(DebuggerClient::OTValues);
  Array values;
  for (ArrayIter iter(m_variables); iter; ++iter) {
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

Array CmdVariable::GetGlobalVariables() {
  Array ret = hhvm
    ? g_vmContext->m_globalVarEnv->getDefinedVariables()
    : get_globals()->getDefinedVars();
  ret.remove("GLOBALS");
  return ret;
}

Array CmdVariable::GetLocalVariables(FrameInjection* frame, bool &global) {
  const_assert(!hhvm);
  Array ret;
  if (!frame || FrameInjection::IsGlobalScope(frame)) {
    global = true;
    ret = GetGlobalVariables();
  } else {
    global = false;
  }
  return ret;
}

bool CmdVariable::onServer(DebuggerProxy *proxy) {
  FrameInjection *frame = FrameInjection::GetStackFrame(m_frame);
  m_variables = GetLocalVariables(frame, m_global);
  return proxy->send(this);
}

bool CmdVariable::onServerVM(DebuggerProxy *proxy) {
  const_assert(hhvm);
  m_variables = g_vmContext->getLocalDefinedVariables(m_frame);
  return proxy->send(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
