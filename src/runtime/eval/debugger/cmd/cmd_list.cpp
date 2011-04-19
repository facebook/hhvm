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

#include <runtime/eval/debugger/cmd/cmd_list.h>
#include <runtime/base/file/file.h>
#include <runtime/ext/ext_file.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdList::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_file);
  thrift.write(m_line1);
  thrift.write(m_line2);
  thrift.write(m_code);
}

void CmdList::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_file);
  thrift.read(m_line1);
  thrift.read(m_line2);
  thrift.read(m_code);
}

void CmdList::list(DebuggerClient *client) {
  client->addCompletion(DebuggerClient::AutoCompleteFileNames);
}

bool CmdList::help(DebuggerClient *client) {
  client->helpTitle("List Command");
  client->helpCmds(
    "list",                   "displays current block of source code",
    "list {line}",            "displays code around specified line",
    "list {line1}-{line2}",   "displays specified block of source code",
    "list {line1}-",          "displays code starting with the line",
    "list -{line2}",          "displays code ending with the line",
    "list {file}",            "displays beginning lines of the file",
    "list {file}:{line}",     "displays code around specified file:line",
    "list {file}:{l1}-{l2}",  "displays specified block in the file",
    "list {file}:{l1}-",      "displays specified block in the file",
    "list {file}:-{l2}",      "displays specified block in the file",
    "list {directory}",       "sets PHP source root directory",
    NULL
  );
  client->helpBody(
    "Use list command to display PHP source code. In remote debugging, this "
    "is displaying source code on server side. When server side cannot find "
    "the file, it will fall back to local files.\n"
    "\n"
    "Hit return to display more lines of code after current display.\n"
    "\n"
    "When a directory name is specified, this will be set to root directory "
    "for resolving relative paths of PHP files. Files with absolute paths "
    "will not be affected by this setting. This directory will be stored "
    "in configuration file for future sessions as well."
  );
  return true;
}

bool CmdList::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;
  if (client->argCount() > 1) {
    return help(client);
  }

  int line = 0;
  m_line1 = m_line2 = 0;
  if (client->argCount() == 1) {
    string arg = client->argValue(1);
    if (DebuggerClient::IsValidNumber(arg)) {
      line = atoi(arg.c_str());
      if (line <= 0) {
        client->error("A line number has to be a positive integer.");
        return help(client);
      }
      m_line1 = line - DebuggerClient::CodeBlockSize/2;
      m_line2 = m_line1 + DebuggerClient::CodeBlockSize;
    } else {
      size_t pos = arg.find(':');
      if (pos != string::npos) {
        m_file = arg.substr(0, pos);
        if (m_file.empty()) {
          client->error("File name cannot be empty.");
          return help(client);
        }
        arg = arg.substr(pos + 1);
      }
      pos = arg.find('-');
      if (pos != string::npos) {
        string line1 = arg.substr(0, pos);
        string line2 = arg.substr(pos + 1);
        if (!DebuggerClient::IsValidNumber(line1) ||
            !DebuggerClient::IsValidNumber(line2)) {
          if (m_file.empty()) {
            m_file = arg;
            m_line1 = 1;
            m_line2 = DebuggerClient::CodeBlockSize;
          } else {
            client->error("Line numbers have to be integers.");
            return help(client);
          }
        } else {
          m_line1 = atoi(line1.c_str());
          m_line2 = atoi(line2.c_str());
          if (line1.empty()) {
            m_line1 = m_line2 - DebuggerClient::CodeBlockSize;
          }
          if (line2.empty()) {
            m_line2 = m_line1 + DebuggerClient::CodeBlockSize;
          }
          if (m_line1 <= 0 || m_line2 <= 0) {
            client->error("Line numbers have to be positive integers.");
            return help(client);
          }
        }
      } else {
        if (!DebuggerClient::IsValidNumber(arg)) {
          if (m_file.empty()) {
            m_file = arg;
            m_line1 = 1;
            m_line2 = DebuggerClient::CodeBlockSize;
          } else {
            client->error("A line number has to be an integer.");
            return help(client);
          }
        } else {
          int line = atoi(arg.c_str());
          if (line <= 0) {
            client->error("A line number has to be a positive integer.");
            return help(client);
          }
          m_line1 = line - DebuggerClient::CodeBlockSize/2;
          m_line2 = m_line1 + DebuggerClient::CodeBlockSize;
        }
      }
    }
  }

  int charFocus0 = 0;
  int lineFocus1 = 0;
  int charFocus1 = 0;

  if (m_file.empty()) {
    int linePrev = 0;
    client->getListLocation(m_file, linePrev, line, charFocus0, lineFocus1,
                            charFocus1);
    if (m_line1 == 0 && m_line2 == 0) {
      m_line1 = linePrev + 1;
      m_line2 = m_line1 + DebuggerClient::CodeBlockSize;
    }
    if (m_file.empty()) {
      string code = client->getCode();
      if (code.empty()) {
        client->error("There is no current source file.");
        return true;
      }
      client->print(highlight_php(code));
      return true;
    }
  } else {
    struct stat sb;
    stat(m_file.c_str(), &sb);
    if ((sb.st_mode & S_IFMT) == S_IFDIR) {
      client->setSourceRoot(m_file);
      client->info("PHP source root directory is set to %s", m_file.c_str());
      return true;
    }
  }

  if (m_line1 <= 0) m_line1 = 1;
  if (m_line2 <= 0) m_line2 = 1;
  if (m_line1 > m_line2) {
    int32 tmp = m_line1;
    m_line1 = m_line2;
    m_line2 = tmp;
  }

  CmdListPtr res = client->xend<CmdList>(this);
  if (res->m_code.isString()) {
    if (!client->code(res->m_code, line, m_line1, m_line2, charFocus0,
                      lineFocus1, charFocus1)) {
      client->info("No more lines in %s to display.", m_file.c_str());
    }
    client->setListLocation(m_file, m_line2, false);
  } else {
    client->error("Unable to read specified source file location.");
  }

  return true;
}

bool CmdList::onServer(DebuggerProxy *proxy) {
  m_code = f_file_get_contents(m_file.c_str());
  return proxy->send(this);
}

Variant CmdList::GetSourceFile(DebuggerClient *client,
                               const std::string &file) {
  CmdList cmd;
  cmd.m_file = file;
  CmdListPtr res = client->xend<CmdList>(&cmd);
  return res->m_code;
}

///////////////////////////////////////////////////////////////////////////////
}}
