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

#include "hphp/runtime/debugger/cmd/cmd_where.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/debugger/debugger_client.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdWhere::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  {
    String sdata;
    DebuggerWireHelpers::WireSerialize(m_stacktrace, sdata);
    thrift.write(sdata);
  }
  thrift.write(m_stackArgs);
}

void CmdWhere::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  {
    String sdata;
    thrift.read(sdata);
    if (DebuggerWireHelpers::WireUnserialize(sdata, m_stacktrace) !=
        DebuggerWireHelpers::NoError) {
      m_stacktrace.reset();
      m_wireError = sdata.toCppString();
    }
  }
  thrift.read(m_stackArgs);
}

void CmdWhere::help(DebuggerClient &client) {
  client.helpTitle("Where Command");
  client.helpCmds(
    "[w]here", "displays current stacktrace",
    "[w]here {num}", "displays number of innermost frames",
    "[w]here -{num}", "displays number of outermost frames",
    nullptr
  );
  client.helpBody(
    "Use '[u]p {num}' or '[d]own {num}' to walk up or down the stacktrace. "
    "Use '[f]rame {index}' to jump to one particular frame. At any frame, "
    "Use '[v]ariable' command to display all local variables."
  );
}

Array CmdWhere::fetchStackTrace(DebuggerClient &client) {
  Array st = client.getStackTrace();
  // Only grab a new stack trace if we don't have one cached.
  if (st.isNull()) {
    m_stackArgs = client.getDebuggerClientStackArgs();
    auto cmd = client.xend<CmdWhere>(this);
    st = cmd->m_stacktrace;
    client.setStackTrace(st);
  }
  return st;
}

void CmdWhere::onClient(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;
  if (client.argCount() > 2) {
    help(client);
    return;
  }
  int argBase = 1;
  if ((client.argCount() > 0) && client.arg(argBase, "async")) {
    // Old clients may send the async modifier, ignore it.
    argBase++;
    client.info("Ignoring async modifier...");
  }

  Array st = fetchStackTrace(client);
  if (st.empty()) {
    client.info("(no stacktrace to display or in global scope)");
    client.info("If you hit the serialization limit, try "
                "\"set sa off\" to get the stack without args");
    return;
  }

  // so list command can default to current frame
  client.moveToFrame(client.getFrame(), false);

  if (client.argCount() < argBase) {
    int i = 0;
    for (ArrayIter iter(st); iter; ++iter) {
      client.printFrame(i, iter.second().toArray());
      ++i;
      if (i % DebuggerClient::ScrollBlockSize == 0 &&
          client.ask("There are %zd more frames. Continue? [Y/n]",
                      st.size() - i) == 'n') {
        break;
      }
    }
  } else {
    std::string snum = client.argValue(argBase);
    int num = atoi(snum.c_str());
    if (snum[0] == '-') {
      snum = snum.substr(1);
    }
    if (!DebuggerClient::IsValidNumber(snum)) {
      client.error("The argument, if specified, has to be numeric.");
      return;
    }
    if (num > 0) {
      for (int i = 0; i < num && i < st.size(); i++) {
        client.printFrame(i, st[i].toArray());
      }
    } else if (num < 0) {
      for (int i = st.size() + num; i < st.size(); i++) {
        client.printFrame(i, st[i].toArray());
      }
    } else {
      client.error("0 was specified for the number of frames");
      client.tutorial(
        "The optional argument is the number of frames to print out. "
        "Use a positive number to print out innermost frames. Use a negative "
        "number to print out outermost frames."
      );
    }
  }
}

bool CmdWhere::onServer(DebuggerProxy &proxy) {
  m_stacktrace = createBacktrace(BacktraceArgs()
                                 .withSelf()
                                 .ignoreArgs(!m_stackArgs));
  return proxy.sendToClient(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
