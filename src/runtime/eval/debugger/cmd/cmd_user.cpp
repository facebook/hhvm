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

#include <runtime/eval/debugger/cmd/cmd_user.h>
#include <runtime/ext/ext_debugger.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdUser::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_cmd);
}

void CmdUser::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_cmd);
}

void CmdUser::list(DebuggerClient *client) {
  Lock lock(s_mutex);
  CmdExtended::list(client);
}

bool CmdUser::help(DebuggerClient *client) {
  client->helpTitle("User Extended Command");
  helpImpl(client, "y");
  client->helpBody(
    "These commands are implemented and installed in PHP by implementing "
    "DebuggerCommand and calling hphpd_install_user_command(). For example\n"
    "\n"
    "  class MyCommand implements DebuggerCommand {\n"
    "    public function help($client) {\n"
    "      $client->helpTitle(\"Hello Command\");\n"
    "      $client->helpCmds(\"y [h]ello\", \"prints welcome message\");\n"
    "      return true;\n"
    "    }\n"
    "    public function onClient($client) {\n"
    "      $client->output(\"Hello, world!\");\n"
    "      return true;\n"
    "    }\n"
    "  }\n"
    "\n"
    "  hphpd_install_user_command('hello', 'MyCommand');\n"
    "\n"
    "Type '[i]nfo DebuggerCommand' for complete DebuggerCommand interface. "
    "Type '[i]nfo DebuggerClient' for complete DebuggerClient interface that "
    "you can use when implementing those $client callbacks. "
    "Type '[i]nfo DebuggerProxy' for complete DebuggerProxy interface that "
    "you can use when implementing those $proxy callbacks."
  );
  return true;
}

const ExtendedCommandMap &CmdUser::getCommandMap() {
  return s_commands;
}

void CmdUser::invokeList(DebuggerClient *client, const std::string &cls) {
  p_DebuggerClient pclient(NEWOBJ(c_DebuggerClient)());
  pclient->m_client = client;
  try {
    Object cmd = create_object(cls.c_str(), null_array);
    cmd->o_invoke("onAutoComplete", CREATE_VECTOR1(pclient), -1);
  } catch (...) {}
}

bool CmdUser::invokeHelp(DebuggerClient *client, const std::string &cls) {
  p_DebuggerClient pclient(NEWOBJ(c_DebuggerClient)());
  pclient->m_client = client;
  try {
    Object cmd = create_object(cls.c_str(), null_array);
    Variant ret = cmd->o_invoke("help", CREATE_VECTOR1(pclient), -1);
    return !same(ret, false);
  } catch (...) {}
  return false;
}

bool CmdUser::invokeClient(DebuggerClient *client, const std::string &cls) {
  p_DebuggerClient pclient(NEWOBJ(c_DebuggerClient)());
  pclient->m_client = client;
  try {
    Object cmd = create_object(cls.c_str(), null_array);
    Variant ret = cmd->o_invoke("onClient", CREATE_VECTOR1(pclient), -1);
    return !same(ret, false);
  } catch (...) {}
  return false;
}

bool CmdUser::onServer(DebuggerProxy *proxy) {
  if (m_cmd.isNull()) return false;
  p_DebuggerProxy pproxy(NEWOBJ(c_DebuggerProxy)());
  pproxy->m_proxy = proxy;
  try {
    Variant ret = m_cmd->o_invoke("onServer", CREATE_VECTOR1(pproxy), -1);
    return !same(ret, false);
  } catch (...) {}
  return false;
}

///////////////////////////////////////////////////////////////////////////////

Mutex CmdUser::s_mutex;
ExtendedCommandMap CmdUser::s_commands;

bool CmdUser::InstallCommand(CStrRef cmd, CStrRef clsname) {
  Lock lock(s_mutex);
  if (s_commands.find(cmd.data()) == s_commands.end()) {
    s_commands[cmd.data()] = string(clsname.data());
    return true;
  }
  return false;
}

Array CmdUser::GetCommands() {
  Lock lock(s_mutex);
  Array ret(Array::Create());
  for (ExtendedCommandMap::const_iterator iter = s_commands.begin();
       iter != s_commands.end(); ++iter) {
    ret.set(String(iter->first), String(iter->second));
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}}
