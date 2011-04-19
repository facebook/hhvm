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

#include <runtime/eval/debugger/cmd/cmd_where.h>
#include <runtime/base/array/array_iterator.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdWhere::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_stacktrace);
}

void CmdWhere::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_stacktrace);
}

bool CmdWhere::help(DebuggerClient *client) {
  client->helpTitle("Where Command");
  client->helpCmds(
    "[w]here",           "displays current stacktrace",
    "[w]here {num}",     "displays number of innermost frames",
    "[w]here -{num}",    "displays number of outermost frames",
    NULL
  );
  client->helpBody(
    "Use '[u]p {num}' or '[d]own {num}' to walk up or down the stacktrace. "
    "Use '[f]rame {index}' to jump to one particular frame. At any frame, "
    "use '[v]ariable' command to display all local variables."
  );
  return true;
}

Array CmdWhere::fetchStackTrace(DebuggerClient *client) {
  Array st = client->getStackTrace();
  if (st.isNull()) {
    CmdWherePtr cmd = client->xend<CmdWhere>(this);
    st = cmd->m_stacktrace;
    client->setStackTrace(st);
  }
  return st;
}

bool CmdWhere::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;
  if (client->argCount() > 1) {
    return help(client);
  }

  Array st = fetchStackTrace(client);
  if (st.empty()) {
    client->info("(no stacktrace to display or in global scope)");
    return true;
  }

  // so list command can default to current frame
  client->moveToFrame(client->getFrame(), false);

  if (client->argCount() == 0) {
    int i = 0;
    for (ArrayIter iter(st); iter; ++iter) {
      client->printFrame(i, iter.second());
      if (++i % DebuggerClient::ScrollBlockSize == 0 &&
          client->ask("There are %d more frames. Continue? [Y/n]",
                      st.size() - i) == 'n') {
        break;
      }
    }
  } else {
    string snum = client->argValue(1);
    int num = atoi(snum.c_str());
    if (snum[0] == '-') {
      snum = snum.substr(1);
    }
    if (!DebuggerClient::IsValidNumber(snum)) {
      client->error("The argument, if specified, has to be numeric.");
      return true;
    }
    if (num > 0) {
      for (int i = 0; i < num && i < st.size(); i++) {
        client->printFrame(i, st[i]);
      }
    } else if (num < 0) {
      for (int i = st.size() + num; i < st.size(); i++) {
        client->printFrame(i, st[i]);
      }
    } else {
      client->error("0 was specified for the number of frames");
      client->tutorial(
        "The optional argument is the number of frames to print out. "
        "Use a positive number to print out innermost frames. Use a negative "
        "number to print out outermost frames."
      );
    }
  }

  return true;
}

bool CmdWhere::onServer(DebuggerProxy *proxy) {
  m_stacktrace = FrameInjection::GetBacktrace(false, false, false);
  return proxy->send(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
