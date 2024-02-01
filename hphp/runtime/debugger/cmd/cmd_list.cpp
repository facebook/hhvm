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

#include "hphp/runtime/debugger/cmd/cmd_list.h"

#include <sys/types.h>
#include <sys/stat.h>

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/debugger/cmd/cmd_info.h"
#include "hphp/runtime/debugger/debugger_client.h"
#include "hphp/runtime/ext/std/ext_std_file.h"

#include <folly/portability/Unistd.h>

namespace HPHP::Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

// Always called from send and implements specific
// logic for serializing a list command to send via Thrift.
void CmdList::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_file);
  thrift.write(m_line1);
  thrift.write(m_line2);
  thrift.write(m_code);
}

// Always called from recv and implements specific
// logic for deserializing a list command received via Thrift.
void CmdList::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_file);
  thrift.read(m_line1);
  thrift.read(m_line2);
  thrift.read(m_code);
}

// Informs the client of all strings that may follow a list command.
// Used for auto completion. The client uses the prefix of the argument
// following the command to narrow down the list displayed to the user.
void CmdList::list(DebuggerClient &client) {
  client.addCompletion(DebuggerClient::AutoCompleteFileNames);
}

// The text to display when the debugger client processes "help list".
void CmdList::help(DebuggerClient &client) {
  client.helpTitle("List Command");
  client.helpCmds(
    "list",                   "displays current block of source code",
    "list {line}",            "displays code around specified line",
    "list {line1}-{line2}",   "displays specified block of source code",
    "list {line1}-",          "displays code starting with the line",
    "list -{line2}",          "displays code ending with the line",
    "list {file}",            "displays beginning lines of the file",
    "list {cls}",             "displays beginning lines of the class",
    "list {function}",        "displays beginning lines of the function",
    "list {cls::method}",     "displays beginning lines of the method",
    "list {file}:{line}",     "displays code around specified file:line",
    "list {file}:{l1}-{l2}",  "displays specified block in the file",
    "list {file}:{l1}-",      "displays specified block in the file",
    "list {file}:-{l2}",      "displays specified block in the file",
    "list {directory}",       "sets PHP source root directory",
    nullptr
  );
  client.helpBody(
    "Use list command to display PHP source code. In remote debugging, this "
    "is displaying source code on server side. When server side cannot find "
    "the file, it will fall back to local files.\n"
    "\n"
    "Hit return to display more lines of code after current display.\n"
    "\n"
    "When a directory name is specified, this will become a root directory "
    "for resolving relative paths of PHP files. Files with absolute paths "
    "will not be affected by this setting. This directory will be stored "
    "in configuration file for future sessions as well."
  );
}

// Retrieves the current source location (file, line).
// The current location is initially determined by the location
// where execution was interrupted to hand control back to
// the debugger client and can thereafter be modified by list
// commands and by switching the stack frame.
//
// The lineFocus and charFocus parameters
// are non zero only when the source location comes from a breakpoint.
// They can be used to highlight the location of the current breakpoint
// in the edit window of an attached IDE, for example.
//
// If m_line1 and m_line2 are currently 0 (because they have not been specified
// as parameters to the list command), they are updated to point to a block of
// code one line beyond the current line maintained by the client.
// This has the effect that a succession of list commands that specify no
// parameters will scroll sequentially through the source code in blocks
// of DebuggerClient::CodeBlockSize.
void CmdList::getListLocation(DebuggerClient &client, int &lineFocus0,
                          int &charFocus0, int &lineFocus1,
                          int &charFocus1) {
  int currentLine = 0;
  client.getListLocation(m_file, currentLine, lineFocus0, charFocus0,
                         lineFocus1, charFocus1);
  if (m_line1 == 0 && m_line2 == 0) {
    m_line1 = currentLine + 1;
    m_line2 = m_line1 + DebuggerClient::CodeBlockSize;
  }
}

// If there is no current file, print the desired range of eval code
// or give an error message if the debugger is not currently performing
// an eval command.
void CmdList::listEvalCode(DebuggerClient &client) {
  assertx(m_file.empty());

  std::string evalCode = client.getCode();
  if (evalCode.empty()) {
    client.error("There is no current source file.");
  } else {
    client.print(highlight_php(evalCode));
  }
}

// Sends this list command to the server to retrieve the source to be listed
// and then displays the source on the client. The client's current line
// is then updated to point to the last listed line.
// Returns false if the server was unable to return source for this command.
bool CmdList::listFileRange(DebuggerClient &client,
                            int lineFocus0, int charFocus0,
                            int lineFocus1, int charFocus1) {
  if (m_line1 <= 0) m_line1 = 1;
  if (m_line2 <= 0) m_line2 = 1;
  if (m_line1 > m_line2) {
    int32_t tmp = m_line1;
    m_line1 = m_line2;
    m_line2 = tmp;
  }

  auto res = client.xend<CmdList>(this);
  if (res->m_code.isString()) {
    if (!client.code(res->m_code.toString(), m_line1, m_line2,
                     lineFocus0, charFocus0, lineFocus1, charFocus1)) {
      client.info("No more lines in %s to display.", m_file.c_str());
    }
    client.setListLocation(m_file, m_line2, false);
    return true;
  }
  return false;
}

const StaticString
  s_methods("methods"),
  s_file("file"),
  s_line1("line1"),
  s_line2("line2");

// Sends an Info command to the server to retrieve source location
// information for the function or class specified by the command
// argument. Then updates this command with the source information
// and sends it to the server in order to retrieve the source
// text from the server.
// Returns false if the server was unable to return the information
// needed for this command.
bool CmdList::listFunctionOrClass(DebuggerClient &client) {
  assertx(client.argCount() == 1);
  auto cmdInfo = std::make_shared<CmdInfo>();
  std::string subsymbol;
  cmdInfo->parseOneArg(client, subsymbol);
  auto cmd = client.xend<CmdInfo>(cmdInfo.get());
  Array info = cmd->getInfo();
  if (info.empty()) return false;
  always_assert(info.size() == 1);
  ArrayIter iter(info);
  Array funcInfo = iter.second().toArray();
  if (!subsymbol.empty()) {
    String key = CmdInfo::FindSubSymbol(funcInfo[s_methods].toArray(),
                                        subsymbol);
    if (key.isNull()) return false;
    funcInfo = funcInfo[s_methods].toArray()[key].toArray();
  }
  String file = funcInfo[s_file].toString();
  auto line1 = (int)funcInfo[s_line1].toInt64();
  auto line2 = (int)funcInfo[s_line2].toInt64();
  int line = line1 ? line1 : line2;
  if (file.empty() || !line) return false;
  client.setListLocation(file.data(), line - 1, false);
  line = 0;
  int charFocus0 = 0;
  int lineFocus1 = 0;
  int charFocus1 = 0;
  m_file.clear();
  m_line1 = m_line2 = 0;
  getListLocation(client, line, charFocus0, lineFocus1, charFocus1);
  if (m_file.empty()) {
    listEvalCode(client);
    return true;
  }
  return listFileRange(client, line, charFocus0, lineFocus1, charFocus1);
}

// Checks the command arguments, report errors and returning as appropriate.
// Then communicates with the server to retrieve source information. Also
// retrieves and updates location information stored in the client.
void CmdList::onClient(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;
  if (client.argCount() > 1) {
    help(client);
    return;
  }

  int line = 0;
  m_line1 = m_line2 = 0;
  if (client.argCount() == 1) {
    std::string arg = client.argValue(1);
    if (DebuggerClient::IsValidNumber(arg)) {
      line = atoi(arg.c_str());
      if (line <= 0) {
        client.error("A line number has to be a positive integer.");
        help(client);
        return;
      }
      m_line1 = line - DebuggerClient::CodeBlockSize/2;
      m_line2 = m_line1 + DebuggerClient::CodeBlockSize;
    } else if (arg.find("::") != std::string::npos) {
      if (!listFunctionOrClass(client)) {
        client.error("Unable to read specified method.");
      }
      return;
    } else {
      size_t pos = arg.find(':');
      if (pos != std::string::npos) {
        m_file = arg.substr(0, pos);
        if (m_file.empty()) {
          client.error("File name cannot be empty.");
          help(client);
          return;
        }
        arg = arg.substr(pos + 1);
      }
      pos = arg.find('-');
      if (pos != std::string::npos) {
        std::string line1 = arg.substr(0, pos);
        std::string line2 = arg.substr(pos + 1);
        if (!DebuggerClient::IsValidNumber(line1) ||
            !DebuggerClient::IsValidNumber(line2)) {
          if (m_file.empty()) {
            m_file = arg;
            m_line1 = 1;
            m_line2 = DebuggerClient::CodeBlockSize;
          } else {
            client.error("Line numbers have to be integers.");
            help(client);
            return;
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
            client.error("Line numbers have to be positive integers.");
            help(client);
            return;
          }
        }
      } else {
        if (!DebuggerClient::IsValidNumber(arg)) {
          if (m_file.empty()) {
            if (client.argCount() == 1 && listFunctionOrClass(client)) {
              return;
            }
            m_file = arg;
            m_line1 = 1;
            m_line2 = DebuggerClient::CodeBlockSize;
          } else {
            client.error("A line number has to be an integer.");
            help(client);
            return;
          }
        } else {
          line = atoi(arg.c_str());
          if (line <= 0) {
            client.error("A line number has to be a positive integer.");
            help(client);
            return;
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
    getListLocation(client, line, charFocus0, lineFocus1, charFocus1);
    if (m_file.empty()) {
      listEvalCode(client);
      return;
    }
  } else if (m_file[0] == '/') {
    struct stat sb;
    stat(m_file.c_str(), &sb);
    if ((sb.st_mode & S_IFMT) == S_IFDIR) {
      client.setSourceRoot(m_file);
      client.info("PHP source root directory is set to %s", m_file.c_str());
      return;
    }
  }

   if (!listFileRange(client, line, charFocus0, lineFocus1, charFocus1)) {
     client.error(
       "Unable to read specified function, class or source file location.");
  }
}

// Tries to read the contents of the file whose path is specified in m_file.
// If the path cannot be resolved and is relative, the path of the sandbox
// is used to qualify the relative path. If the contents cannot be retrieved
// m_code will be an empty string.
// The function returns false if the reply to the client fails during the
// sending process.
bool CmdList::onServer(DebuggerProxy &proxy) {
  auto savedWarningFrequency = RuntimeOption::WarningFrequency;
  RuntimeOption::WarningFrequency = 0;
  m_code = HHVM_FN(file_get_contents)(m_file.c_str());
  if (!proxy.isLocal() && !m_code.toBoolean() && m_file[0] != '/') {
    DSandboxInfo info = proxy.getSandbox();
    if (info.m_path.empty()) {
      raise_warning("path for sandbox %s is not setup, run a web request",
                    info.desc().c_str());
    } else {
      std::string full_path = info.m_path + m_file;
      m_code = HHVM_FN(file_get_contents)(full_path.c_str());
    }
  }
  RuntimeOption::WarningFrequency = savedWarningFrequency;
  if (!m_code.toBoolean() && FileUtil::isSystemName(m_file)) {
    m_code = SystemLib::s_source;
  }
  return proxy.sendToClient((DebuggerCommand*)this);
}

// Sends a "list file" command to the proxy attached to the given client.
// Returns false if the file does not exist or could not be read as an
// HPHP::String instance containing the contents of the file.
Variant CmdList::GetSourceFile(DebuggerClient &client,
                               const std::string &file) {
  CmdList cmd;
  cmd.m_file = file;
  auto res = client.xend<CmdList>(&cmd);
  return res->m_code;
}

///////////////////////////////////////////////////////////////////////////////
}
