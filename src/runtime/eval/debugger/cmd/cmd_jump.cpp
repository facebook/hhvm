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

#include <runtime/eval/debugger/cmd/cmd_jump.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdJump::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_label);
  thrift.write(m_file);
  thrift.write(m_line);
}

void CmdJump::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_label);
  thrift.read(m_file);
  thrift.read(m_line);
}

void CmdJump::list(DebuggerClient *client) {
  client->addCompletion(DebuggerClient::AutoCompleteFileNames);
}

bool CmdJump::help(DebuggerClient *client) {
  client->helpTitle("Jump Command");
  client->helpCmds(
    "[j]ump",                 "jumps over one expression",
    "[j]ump {line}",          "goto the specified line",
    "[j]ump {file}:{line}",   "goto the specified line",
    "[j]ump {label}",         "goto the specified label",
    NULL
  );
  client->helpBody(
    "This command changes program execution to the specified place without "
    "executing remaining code on current line. When no label or source "
    "location is specified, it jumps over just one expression without "
    "evaluating it. This may be useful to not throw an exception when "
    "breaks at a throw, for example."
  );
  return true;
}

bool CmdJump::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;
  if (client->argCount() > 1) {
    return help(client);
  }

  string arg = client->argValue(1);
  m_line = 0;
  if (!arg.empty()) {
    if (DebuggerClient::IsValidNumber(arg)) {
      BreakPointInfoPtr loc = client->getCurrentLocation();
      if (loc) {
        m_file = loc->m_file;
      }
      if (m_file.empty()) {
        client->error("There is no current source file. Please specify a file "
                      "name as well.");
        return true;
      }
      m_line = atoi(arg.c_str());
    } else {
      size_t pos = arg.find(':');
      if (pos == string::npos) {
        m_label = arg;
      } else {
        m_file = arg.substr(0, pos);
        m_line = atoi(arg.substr(pos + 1).c_str());
        if (m_file.empty() || m_line <= 0) {
          client->error("Invalid source location. Cannot jump there.");
          return true;
        }
      }
    }
  }

  client->send(this);
  throw DebuggerConsoleExitException();
}

bool CmdJump::onServer(DebuggerProxy *proxy) {
  return true;
}

bool CmdJump::match(InterruptSite *site) {
  if (site) {
    if (m_line) {
      if (m_line == site->getLine0()) {
        const char *file = site->getFile();
        return BreakPointInfo::MatchFile(file, strlen(file), m_file);
      }
    } else {
      // label is not implemented
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}}
