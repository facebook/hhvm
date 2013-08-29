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

#include "hphp/runtime/debugger/cmd/cmd_extension.h"
#include "hphp/runtime/ext/ext_array.h"
#include "hphp/util/text-art.h"

using namespace HPHP::Util::TextArt;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdExtension::sendImpl(DebuggerThriftBuffer &thrift) {
  CmdExtended::sendImpl(thrift);
  thrift.write(m_args);
  thrift.write(m_out);
  thrift.write(m_err);
}

void CmdExtension::recvImpl(DebuggerThriftBuffer &thrift) {
  CmdExtended::recvImpl(thrift);
  thrift.read(m_args);
  thrift.read(m_out);
  thrift.read(m_err);
}

void CmdExtension::list(DebuggerClient &client) {
  if (client.argCount() == 2) {
    client.addCompletion("dump");
  } else if (client.argCount() > 2) {
    client.addCompletion(DebuggerClient::AutoCompleteFileNames);
  } else {
    // This is cheating, assuming server has same list of extensions.
    Array exts = Extension::GetLoadedExtensions();
    vector<std::string> items;
    for (ArrayIter iter(exts); iter; ++iter) {
      items.push_back(iter.second().toString()->toCPPString());
    }
    client.addCompletion(items);
  }
}

void CmdExtension::help(DebuggerClient &client) {
  client.helpTitle("Extension Command");
  client.helpCmds(
    "x [t]ension",                 "lists all extensions",
    "x [t]ension {name}",          "shows summary info of the extension",
    "x [t]ension {name} dump",     "shows detailed info of the extension",
    "x [t]ension {name} {verb} {args} ...",   "executes an action",
    nullptr
  );
  client.helpBody(
    "In PHP, a lot of library functions are implemented as \"extensions\". "
    "This command allows extensions to support debugger by providing their "
    "version numbers, current status and cached data and by providing "
    "additional verbs to update runtime states for debugging purposes."
  );
}

void CmdExtension::onClient(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;
  m_args = *client.args();
  CmdExtensionPtr cmd = client.xend<CmdExtension>(this);
  if (cmd->m_out.empty()) {
    client.error(cmd->m_err);
  } else {
    client.print(cmd->m_out);
  }
}

bool CmdExtension::processList(DebuggerProxy &proxy) {
  IDebuggable::InfoVec info;

  Array exts = Extension::GetLoadedExtensions();
  typedef std::set<std::string, string_lessi> sorted_iset;
  sorted_iset names;
  for (ArrayIter iter(exts); iter; ++iter) {
    names.insert(iter.second().toString().data());
  }
  for (sorted_iset::const_iterator iter = names.begin();
       iter != names.end(); ++iter) {
    Extension *ext = Extension::GetExtension(*iter);
    assert(ext);
    if (ext) {
      int support = ext->debuggerSupport();
      string line;
      line += (support & IDebuggable::SupportInfo) ? "Yes     " : "        ";
      line += (support & IDebuggable::SupportDump) ? "Yes     " : "        ";
      line += (support & IDebuggable::SupportVerb) ? "Yes     " : "        ";
      line += ext->getVersion();
      IDebuggable::Add(info, iter->c_str(), line);
    }
  }
  int nameLen;
  String body = DebuggerClient::FormatInfoVec(info, &nameLen);
  int hrLen = nameLen + 42;
  if (hrLen > DebuggerClient::LineWidth) hrLen = DebuggerClient::LineWidth;

  StringBuffer sb;
  for (int i = 0; i < hrLen; i++) sb.append(BOX_H); sb.append("\n");
  sb.append(StringUtil::Pad("Name\\Support", nameLen));
  sb.append("Info    Dump    Verb    Version\n");
  for (int i = 0; i < hrLen; i++) sb.append(BOX_H); sb.append("\n");
  sb.append(body);
  for (int i = 0; i < hrLen; i++) sb.append(BOX_H); sb.append("\n");

  m_out = sb.detach();
  return proxy.sendToClient(this);
}

bool CmdExtension::onServer(DebuggerProxy &proxy) {
  if (m_args.size() <= 1) {
    return processList(proxy);
  }

  string name = m_args[1];
  Extension *ext = Extension::GetExtension(name);
  if (ext) {
    if (m_args.size() == 2) {
      if (ext->debuggerSupport() & IDebuggable::SupportInfo) {
        IDebuggable::InfoVec info;
        ext->debuggerInfo(info);
        m_out = DebuggerClient::FormatInfoVec(info);
      } else {
        m_err = "Extension doesn't have summary information.";
      }
    } else if (DebuggerClient::Match(m_args[2].c_str(), "dump")) {
      if (ext->debuggerSupport() & IDebuggable::SupportDump) {
        m_out = ext->debuggerDump();
        m_out += "\n";
      } else {
        m_err = "Extension doesn't have detailed dumps.";
      }
    } else {
      if (ext->debuggerSupport() & IDebuggable::SupportVerb) {
        string verb = m_args[2];
        StringVec args;
        if (m_args.size() > 3) {
          args.insert(args.end(), m_args.begin() + 3, m_args.end());
        }
        m_out = ext->debuggerVerb(verb, args);
        m_out += "\n";
      } else {
        m_err = "Extension doesn't support any debugger actions.";
      }
    }
  } else {
    m_err = "Unable to find the specified extension: ";
    m_err += String(name);
  }
  return proxy.sendToClient(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
