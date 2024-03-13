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

#include "hphp/runtime/debugger/cmd/cmd_variable.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/debugger/cmd/cmd_where.h"
#include "hphp/runtime/debugger/debugger_client.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP::Eval {
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
    thrift.write(m_formatMaxLen);
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
      m_variables.reset();
      if (error != DebuggerWireHelpers::HitLimit || m_version == 0) {
        // Unexpected error. Log it.
        m_wireError = sdata.toCppString();
      }
    }
  }
  thrift.read(m_global);
  if (m_version == 2) {
    thrift.read(m_formatMaxLen);
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

const StaticString
  s_HTTP_RAW_POST_DATA("HTTP_RAW_POST_DATA"),
  s_omitted("...(omitted)");

void CmdVariable::PrintVariable(DebuggerClient &client, const String& varName) {
  CmdVariable cmd;
  auto charCount = client.getDebuggerClientShortPrintCharCount();
  cmd.m_frame = client.getFrame();
  auto rcmd = client.xend<CmdVariable>(&cmd);

  always_assert(rcmd->m_version == 2);

  // Using the new protocol.  rcmd contains a list of variables only.  Fetch
  // value of varName only, so that we can recover nicely when its value is too
  // large to serialize.
  cmd.m_varName = varName;
  cmd.m_variables.reset();
  cmd.m_formatMaxLen = charCount;
  cmd.m_version = 2;
  rcmd = client.xend<CmdVariable>(&cmd);

  if (rcmd->m_variables.empty()) {
    // Perhaps the value is too large? See recvImpl.  Retry the command with
    // version 1, in which case values are omitted.
    cmd.m_version = 1;
    rcmd = client.xend<CmdVariable>(&cmd);
    if (!rcmd->m_variables.empty()) {
      // It's there without values, and gone with values, so it is too large.
      client.output(s_omitted);
    }
    return;
  }

  auto const get_var = [varName] (const CmdVariable& cmd) {
    assertx(cmd.m_variables.size() == 1);
    assertx(cmd.m_variables.exists(varName, true /* isKey */));
    assertx(cmd.m_variables[varName].isString());
    return cmd.m_variables[varName].toString();
  };

  auto const value = get_var(*rcmd);
  if (charCount <= 0 || value.size() <= charCount) {
    client.output(value);
    return;
  }

  // Don't show the "omitted" suffix.
  auto piece = folly::StringPiece{value.data(), static_cast<size_t>(charCount)};
  client.output(piece);
  if (client.ask("There are more characters. Continue? [y/N]") == 'y') {
    // Now we get the full value, and show the rest.
    cmd.m_variables.reset();
    cmd.m_formatMaxLen = -1;
    rcmd = client.xend<CmdVariable>(&cmd);

    auto value_2 = get_var(*rcmd);
    auto rest = folly::StringPiece {
      value_2.data() + charCount,
      static_cast<size_t>(value_2.size() - charCount)
    };
    client.output(rest);
    client.tutorial("You can use 'set cc n' to increase the character"
                    " limit. 'set cc -1' will remove the limit.");
  }
}

void CmdVariable::PrintVariables(DebuggerClient &client, const Array& variables,
                                 int frame, const String& text, int version) {
  bool global = frame == -1; // I.e. we were called from CmdGlobal, or the
  //client's current frame is the global frame, according to OnServer
  bool system = true;
  int i = 0;
  bool found = false;

  always_assert(version == 2);

  for (ArrayIter iter(variables); iter; ++iter) {
    auto const name = iter.first().toString();
    String value;

    // Using the new protocol, so variables contain only names.  Fetch the value
    // separately.
    CmdVariable cmd;
    cmd.m_frame = frame;
    cmd.m_variables.reset();
    cmd.m_varName = name;
    cmd.m_filter = text;
    cmd.m_formatMaxLen = 200;
    cmd.m_version = 2;
    auto rcmd = client.xend<CmdVariable>(&cmd);
    if (!rcmd->m_variables.empty()) {
      assertx(rcmd->m_variables[name].isString());
      value = rcmd->m_variables[name].toString();
      found = true;
    } else if (text.empty()) {
      // Not missing because filtered out, assume the value is too large.
      value = s_omitted;
      found = true;
    } else if (name.find(text, 0, false) >= 0) {
      // Server should have matched it.  Assume missing because value is too
      // large.
      value = s_omitted;
      found = true;
    } else {
      // The variable was filtered out on the server, using text.  Or it was
      // just too large.  Either way we skip over it.
      continue;
    }

    if (global && system) {
      client.print("$%s = %s", name.data(), value.data());
    } else {
      client.output("$%s = %s", name.data(), value.data());
    }

    // We know s_HTTP_RAW_POST_DATA is the last system global.
    if (global && name == s_HTTP_RAW_POST_DATA) {
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

  m_frame = client.getFrame();

  auto cmd = client.xend<CmdVariable>(this);
  if (cmd->m_variables.empty()) {
    client.info("(no variable was defined)");
  } else {
    PrintVariables(client, cmd->m_variables, cmd->m_global ? -1 : m_frame,
        text, cmd->m_version);
  }
}

const StaticString s_GLOBALS("GLOBALS");
const StaticString s_this("this");

Array CmdVariable::GetGlobalVariables() {
  Array ret = getDefinedVariables();
  ret.remove(s_GLOBALS);
  return ret;
}

bool CmdVariable::onServer(DebuggerProxy &proxy) {
  if (m_frame >= 0) {
    m_variables = g_context->getLocalDefinedVariablesDebugger(m_frame);
    auto oThis = g_context->getThis();
    if (nullptr != oThis) {
      auto tvThis = make_tv<KindOfObject>(oThis);
      Variant thisName(s_this);
      m_variables.set(thisName, tvAsVariant(&tvThis));
    }
  }

  auto const& denv = g_context->getDebuggerEnv();
  if (m_frame >= 0 && !denv.isNull()) {
    IterateKV(denv.get(), [&] (TypedValue k, TypedValue v) {
      if (!m_variables.exists(k)) m_variables.set(k, v, true);
    });
  }

  // Deprecated IDE uses this, so keep it around for now.  It expects all
  // variable values to be fully serialized across the wire.
  if (m_version == 0) {
    return proxy.sendToClient(this);
  }

  // Version 1 of this command means we want the names of all variables, but we
  // don't care about their values just yet.
  if (m_version == 1) {
    DictInit ret(m_variables->size());
    Variant v;
    for (ArrayIter iter(m_variables); iter; ++iter) {
      assertx(iter.first().isString());
      ret.set(iter.first().toString(), v);
    }
    m_variables = ret.toArray();
    m_version = 2;
    return proxy.sendToClient(this);
  }

  // Version 2 of this command means we're trying to get the value of a single
  // variable.
  always_assert(m_version == 2);
  always_assert(!m_varName.empty());

  // Variable name might not exist.
  if (!m_variables.exists(m_varName, true /* isKey */)) {
    m_variables = Array::CreateDict();
    return proxy.sendToClient(this);
  }

  auto const value = m_variables[m_varName];
  auto const result = m_formatMaxLen < 0
    ? DebuggerClient::FormatVariable(value)
    : DebuggerClient::FormatVariableWithLimit(value, m_formatMaxLen);
  m_variables = make_dict_array(m_varName, result);

  // Remove the entry if its name or context does not match the filter.
  if (!m_filter.empty() && m_varName.find(m_filter, 0, false) < 0) {
    auto const fullvalue = DebuggerClient::FormatVariable(value);
    if (fullvalue.find(m_filter, 0, false) < 0) {
      m_variables = Array::CreateDict();
    }
  }

  return proxy.sendToClient(this);
}

///////////////////////////////////////////////////////////////////////////////
}
