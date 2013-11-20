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
#include "hphp/runtime/debugger/cmd/cmd_variable.h"

#include "hphp/runtime/base/hphp-system.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/debugger/cmd/cmd_where.h"
#include "hphp/runtime/ext/ext_asio.h"
#include "hphp/runtime/ext/ext_continuation.h"
#include "hphp/runtime/ext/asio/async_function_wait_handle.h"
#include "hphp/runtime/ext/asio/waitable_wait_handle.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdVariable::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_frame);
  {
    String sdata;
    DebuggerWireHelpers::WireSerialize(m_variables, sdata);
    thrift.write(sdata);
  }
  thrift.write(m_global);
  if (m_version == 2) {
    thrift.write(m_varName);
    thrift.write(m_filter);
  }
}

void CmdVariable::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_frame);
  {
    String sdata;
    thrift.read(sdata);
    auto error = DebuggerWireHelpers::WireUnserialize(sdata, m_variables);
    if (error != DebuggerWireHelpers::NoError) {
      m_variables = null_array;
      if (error != DebuggerWireHelpers::HitLimit || m_version == 0) {
        // Unexpected error. Log it.
        m_wireError = sdata;
      }
    }
  }
  thrift.read(m_global);
  if (m_version == 2) {
    thrift.read(m_varName);
    thrift.read(m_filter);
  }
}

void CmdVariable::help(DebuggerClient &client) {
  client.helpTitle("Variable Command");
  client.helpCmds(
    "[v]ariable",           "lists all local variables on stack",
    "[v]ariable {text}",    "full-text search local variables",
    nullptr
  );
  client.helpBody(
    "This will print names and values of all variables that are currently "
    "accessible by simple names. Use '[w]here', '[u]p {num}', '[d]own {num}', "
    "'[f]rame {index}' commands to choose a different frame to view variables "
    "at different level of the stack.\n"
    "\n"
    "Specify some free text to print local variables that contain the text "
    "either in their names or values. The search is case-insensitive and "
    "string-based."
  );
}

void CmdVariable::PrintVariable(DebuggerClient &client, const String& varName) {
  CmdVariable cmd(client.isStackTraceAsync()
                  ? KindOfVariableAsync : KindOfVariable);
  auto charCount = client.getDebuggerClientShortPrintCharCount();
  cmd.m_frame = client.getFrame();
  CmdVariablePtr rcmd = client.xend<CmdVariable>(&cmd);
  if (rcmd->m_version == 2) {
    // Using the new protocol. rcmd contains a list of variables only.
    // Fetch value of varName only, so that we can recover nicely when its
    // value is too large to serialize.
    cmd.m_varName = varName;
    cmd.m_variables.reset();
    cmd.m_version = 2;
    rcmd = client.xend<CmdVariable>(&cmd);
    if (rcmd->m_variables.empty()) {
      // Perhaps the value is too large? See recvImpl.
      // Retry the command with version 1, in which case values are omitted.
      cmd.m_version = 1;
      rcmd = client.xend<CmdVariable>(&cmd);
      if (!rcmd->m_variables.empty()) {
        // It's there without values, and gone with values, so it is too large.
        client.output("...(omitted)");
        return;
      }
    }
  }
  if (!rcmd->m_variables.empty()) {
    for (ArrayIter iter(rcmd->m_variables); iter; ++iter) {
      String name = iter.first().toString();
      if (!name.equal(varName)) continue;
      String value = DebuggerClient::FormatVariable(iter.second(), -1);
      auto excess = value.length() - charCount;
      if (charCount <= 0 || excess <= 0) {
        client.output("%s", value.data());
      } else {
        client.output("%s", value.substr(0, charCount).data());
        if (client.ask("There are %d more characters. Continue? [y/N]", excess)
            == 'y') {
          client.output("%s", value.substr(charCount).data());
          client.tutorial("You can use 'set cc n' to increase the character"
              " limit. 'set cc -1' will remove the limit.");
        }
      }
    }
  }
}

const StaticString s_http_response_header("http_response_header");
const StaticString s_omitted("...(omitted)");


void CmdVariable::PrintVariables(DebuggerClient &client, CArrRef variables,
                                 int frame, const String& text, int version) {
  bool global = frame == -1; // I.e. we were called from CmdGlobal, or the
  //client's current frame is the global frame, according to OnServer
  bool system = true;
  int i = 0;
  bool found = false;
  for (ArrayIter iter(variables); iter; ++iter) {
    String name = iter.first().toString();
    String value;
    if (version == 2) {
      // Using the new protocol, so variables contain only names.
      // Fetch the value separately.
      CmdVariable cmd(client.isStackTraceAsync()
        ? KindOfVariableAsync : KindOfVariable);
      cmd.m_frame = frame;
      cmd.m_variables = null_array;
      cmd.m_varName = name;
      cmd.m_filter = text;
      cmd.m_version = 2;
      auto rcmd = client.xend<CmdVariable>(&cmd);
      if (!rcmd->m_variables.empty()) {
        value = DebuggerClient::FormatVariable(rcmd->m_variables[name], 200);
        found = true;
      } else if (text.empty()) {
        // Not missing because filtered out, assume the value is too large.
        value = s_omitted;
        found = true;
      } else {
        if (name.find(text, 0, false) >= 0) {
          // Server should have matched it.
          // Assume missing because value is too large.
          value = s_omitted;
          found = true;
        } else {
          // The variable was filtered out on the server, using text.
          // Or it was just too large. Either way we let skip over it.
          continue;
        }
      }
    } else {
      value = DebuggerClient::FormatVariable(iter.second(), 200);
    }
    if (version == 0 && !text.empty()) {
      if (name.find(text, 0, false) >= 0) {
        client.print("%s = %s", name.data(), value.data());
        found = true;
      } else {
        String fullvalue = DebuggerClient::FormatVariable(value, -1);
        if (fullvalue.find(text, 0, false) >= 0) {
          client.print("%s = %s", name.data(), value.data());
          found = true;
        }
      }
    } else {
      if (global && system) {
        client.print("$%s = %s", name.data(), value.data());
      } else {
        client.output("$%s = %s", name.data(), value.data());
      }

      // we know s_http_response_header is the last system global
      if (global && name == s_http_response_header) {
        client.output("%s", "");
        system = false;
      }

      ++i;
      if (i % DebuggerClient::ScrollBlockSize == 0 &&
          client.ask("There are %zd more variables. Continue? [Y/n]",
                      variables.size() - i) == 'n') {
        break;
      }
    }
  }

  if (!text.empty() && !found) {
    client.info("(unable to find specified text in any variables)");
  }
}

void CmdVariable::onClient(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;

  String text;
  if (client.argCount() == 1) {
    text = client.argValue(1);
  } else if (client.argCount() != 0) {
    help(client);
    return;
  }

  if (client.isStackTraceAsync()) {
    m_type = KindOfVariableAsync;
  }

  m_frame = client.getFrame();

  CmdVariablePtr cmd = client.xend<CmdVariable>(this);
  if (cmd->m_variables.empty()) {
    client.info("(no variable was defined)");
  } else {
    PrintVariables(client, cmd->m_variables, cmd->m_global ? -1 : m_frame,
        text, cmd->m_version);
  }
}

const StaticString s_GLOBALS("GLOBALS");

Array CmdVariable::GetGlobalVariables() {
  Array ret = g_vmContext->m_globalVarEnv->getDefinedVariables();
  ret.remove(s_GLOBALS);
  return ret;
}

static c_WaitableWaitHandle *objToWaitableWaitHandle(Object o) {
  assert(o->instanceof(c_WaitableWaitHandle::classof()));
  return static_cast<c_WaitableWaitHandle*>(o.get());
}

static c_AsyncFunctionWaitHandle *objToContinuationWaitHandle(Object o) {
  if (o->instanceof(c_AsyncFunctionWaitHandle::classof())) {
    return static_cast<c_AsyncFunctionWaitHandle*>(o.get());
  }
  return nullptr;
}

static
c_AsyncFunctionWaitHandle *getWaitHandleAtAsyncStackPosition(int position) {
  auto top = f_asio_get_running();

  if (top.isNull()) {
    return nullptr;
  }

  if (position == 0) {
    return objToContinuationWaitHandle(top);
  }

  Array depStack =
    objToWaitableWaitHandle(top)->t_getdependencystack();

  return objToContinuationWaitHandle(depStack[position].toObject());
}

static Array getVariables(const ActRec *fp) {
  if (fp->hasVarEnv()) {
    return fp->m_varEnv->getDefinedVariables();
  } else {
    const Func *func = fp->m_func;
    auto numLocals = func->numNamedLocals();
    ArrayInit ret(numLocals);
    for (Id id = 0; id < numLocals; ++id) {
      TypedValue* ptv = frame_local(fp, id);
      if (ptv->m_type == KindOfUninit) {
        continue;
      }
      Variant name(func->localVarName(id));
      ret.add(name, tvAsVariant(ptv));
    }
    return ret.toArray();
  }
}

bool CmdVariable::onServer(DebuggerProxy &proxy) {
  if (m_type == KindOfVariableAsync) {
    //we only do variable inspection on continuation wait handles
    auto frame = getWaitHandleAtAsyncStackPosition(m_frame);

    if (frame != nullptr) {
      auto fp = frame->getActRec();
      if (fp != nullptr) {
        m_variables = getVariables(fp);
      }
    }
  }
  else if (m_frame < 0) {
    m_variables = g_vmContext->m_globalVarEnv->getDefinedVariables();
    m_global = true;
  } else {
    m_variables = g_vmContext->getLocalDefinedVariables(m_frame);
    m_global = g_vmContext->getVarEnv(m_frame) == g_vmContext->m_globalVarEnv;
  }

  if (m_global) {
    m_variables.remove(s_GLOBALS);
  }
  if (m_version == 1) {
    // Remove the values before sending to client.
    ArrayInit ret(m_variables->size());
    Variant v;
    for (ArrayIter iter(m_variables); iter; ++iter) {
      ret.add(iter.first().toString(), v);
    }
    m_variables = ret.toArray();
    m_version = 2;
  } else if (m_version == 2) {
    // Remove entries that do not match a non empty m_varName.
    if (!m_varName.empty()) {
      ArrayInit ret(1);
      ret.add(m_varName, m_variables[m_varName]);
      m_variables = ret.toArray();
    }
    // Remove entries whose name or contents do not match a non empty m_filter
    if (!m_filter.empty()) {
      ArrayInit ret(1);
      for (ArrayIter iter(m_variables); iter; ++iter) {
        String name = iter.first().toString();
        if (name.find(m_filter, 0, false) < 0) {
          String fullvalue = DebuggerClient::FormatVariable(iter.second(), -1);
          if (fullvalue.find(m_filter, 0, false) < 0) {
            continue;
          }
        }
        ret.add(name, iter.second());
      }
      m_variables = ret.toArray();
    }
  }

  return proxy.sendToClient(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
