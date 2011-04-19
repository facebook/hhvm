/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include <runtime/ext/ext_debugger.h>
#include <runtime/ext/ext_string.h>
#include <runtime/eval/debugger/cmd/cmd_user.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

using namespace Eval;

const int64 q_DebuggerClient_AUTO_COMPLETE_FILENAMES =
  DebuggerClient::AutoCompleteFileNames;
const int64 q_DebuggerClient_AUTO_COMPLETE_VARIABLES =
  DebuggerClient::AutoCompleteVariables;
const int64 q_DebuggerClient_AUTO_COMPLETE_CONSTANTS =
  DebuggerClient::AutoCompleteConstants;
const int64 q_DebuggerClient_AUTO_COMPLETE_CLASSES   =
  DebuggerClient::AutoCompleteClasses;
const int64 q_DebuggerClient_AUTO_COMPLETE_FUNCTIONS =
  DebuggerClient::AutoCompleteFunctions;
const int64 q_DebuggerClient_AUTO_COMPLETE_CLASS_METHODS =
  DebuggerClient::AutoCompleteClassMethods;
const int64 q_DebuggerClient_AUTO_COMPLETE_CLASS_PROPERTIES =
  DebuggerClient::AutoCompleteClassProperties;
const int64 q_DebuggerClient_AUTO_COMPLETE_CLASS_CONSTANTS =
  DebuggerClient::AutoCompleteClassConstants;
const int64 q_DebuggerClient_AUTO_COMPLETE_KEYWORDS =
  DebuggerClient::AutoCompleteKeyword;
const int64 q_DebuggerClient_AUTO_COMPLETE_CODE =
  DebuggerClient::AutoCompleteCode;

///////////////////////////////////////////////////////////////////////////////

bool f_hphpd_install_user_command(CStrRef cmd, CStrRef clsname) {
  return CmdUser::InstallCommand(cmd, clsname);
}

Array f_hphpd_get_user_commands() {
  return CmdUser::GetCommands();
}

void f_hphpd_break(bool condition /* = true */) {
  if (RuntimeOption::EnableDebugger && condition) {
    ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
    FrameInjection *frame = FrameInjection::GetStackFrame(1);
    if (frame && ti->m_reqInjectionData.debugger) {
      Eval::InterruptSite site(frame);
      Eval::Debugger::InterruptHard(site);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

c_DebuggerProxy::c_DebuggerProxy() {
}

c_DebuggerProxy::~c_DebuggerProxy() {
}

void c_DebuggerProxy::t___construct() {
}

bool c_DebuggerProxy::t_islocal() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerProxy, DebuggerProxy::islocal);
  return m_proxy->isLocal();
}

Variant c_DebuggerProxy::t_send(CObjRef cmd) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerProxy, DebuggerProxy::send);
  CmdUser cmdUser(cmd);
  return m_proxy->send(&cmdUser);
}

Variant c_DebuggerProxy::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerProxy, DebuggerProxy::__destruct);
  return null;
}

///////////////////////////////////////////////////////////////////////////////

c_DebuggerClient::c_DebuggerClient() {
}

c_DebuggerClient::~c_DebuggerClient() {
}

void c_DebuggerClient::t___construct() {
}

void c_DebuggerClient::t_quit() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::quit);
  m_client->quit();
}

static String format_string(DebuggerClient *client,
                            int _argc, CStrRef format, CArrRef _argv) {
  Variant ret = f_sprintf(_argc, format, _argv);
  if (ret.isString()) {
    return ret;
  }
  client->error("Debugger extension failed to format string: %s",
                 format.data());
  return "";
}

void c_DebuggerClient::t_print(int _argc, CStrRef format,
                               CArrRef _argv /* = null_array */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::print);
  m_client->print(format_string(m_client, _argc, format, _argv));
}

void c_DebuggerClient::t_help(int _argc, CStrRef format,
                              CArrRef _argv /* = null_array */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::help);
  m_client->help(format_string(m_client, _argc, format, _argv));
}

void c_DebuggerClient::t_info(int _argc, CStrRef format,
                              CArrRef _argv /* = null_array */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::info);
  m_client->info(format_string(m_client, _argc, format, _argv));
}

void c_DebuggerClient::t_output(int _argc, CStrRef format,
                                CArrRef _argv /* = null_array */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::output);
  m_client->output(format_string(m_client, _argc, format, _argv));
}

void c_DebuggerClient::t_error(int _argc, CStrRef format,
                               CArrRef _argv /* = null_array */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::error);
  m_client->error(format_string(m_client, _argc, format, _argv));
}

void c_DebuggerClient::t_code(CStrRef source, int highlight_line /* = 0 */,
                              int start_line_no /* = 0 */,
                              int end_line_no /* = 0 */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::code);
  m_client->code(source, highlight_line, start_line_no, end_line_no);
}

Variant c_DebuggerClient::t_ask(int _argc, CStrRef format,
                                CArrRef _argv /* = null_array */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::ask);
  String ret = format_string(m_client, _argc, format, _argv);
  return String::FromChar(m_client->ask("%s", ret.data()));
}

String c_DebuggerClient::t_wrap(CStrRef str) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::wrap);
  return m_client->wrap(str.data());
}

void c_DebuggerClient::t_helptitle(CStrRef str) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::helptitle);
  m_client->helpTitle(str.data());
}

void c_DebuggerClient::t_helpcmds(int _argc, CStrRef cmd, CStrRef desc,
                                  CArrRef _argv /* = null_array */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::helpcmds);
  std::vector<String> holders;
  std::vector<const char *> cmds;
  cmds.push_back(cmd.data());
  cmds.push_back(desc.data());
  for (int i = 0; i < _argv.size(); i++) {
    String s = _argv[i].toString();
    holders.push_back(s);
    cmds.push_back(s.data());
  }
  m_client->helpCmds(cmds);
}

void c_DebuggerClient::t_helpbody(CStrRef str) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::helpbody);
  m_client->helpBody(str.data());
}

void c_DebuggerClient::t_helpsection(CStrRef str) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::helpsection);
  m_client->helpSection(str.data());
}

void c_DebuggerClient::t_tutorial(CStrRef str) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::tutorial);
  m_client->tutorial(str.data());
}

String c_DebuggerClient::t_getcode() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::getcode);
  return m_client->getCode();
}

String c_DebuggerClient::t_getcommand() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::getcommand);
  return m_client->getCommand();
}

bool c_DebuggerClient::t_arg(int index, CStrRef str) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::arg);
  return m_client->arg(index + 1, str.data());
}

int c_DebuggerClient::t_argcount() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::argcount);
  return m_client->argCount() - 1;
}

String c_DebuggerClient::t_argvalue(int index) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::argvalue);
  return m_client->argValue(index + 1);
}

String c_DebuggerClient::t_argrest(int index) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::argrest);
  return m_client->argRest(index + 1);
}

Array c_DebuggerClient::t_args() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::args);
  StringVec *args = m_client->args();
  Array ret(Array::Create());
  for (unsigned int i = 1; i < args->size(); i++) {
    ret.append(String(args->at(i)));
  }
  return ret;
}

Variant c_DebuggerClient::t_send(CObjRef cmd) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::send);
  CmdUser cmdUser(cmd);
  m_client->send(&cmdUser);
  return true;
}

Variant c_DebuggerClient::t_xend(CObjRef cmd) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::xend);
  CmdUser cmdUser(cmd);
  CmdUserPtr ret = m_client->xend<CmdUser>(&cmdUser);
  return ret->getUserCommand();
}

Variant c_DebuggerClient::t_getcurrentlocation() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::getcurrentlocation);
  BreakPointInfoPtr bpi = m_client->getCurrentLocation();
  Array ret(Array::Create());
  if (bpi) {
    ret.set("file",      String(bpi->m_file));
    ret.set("line",      (int64)bpi->m_line1);
    ret.set("namespace", String(bpi->getNamespace()));
    ret.set("class",     String(bpi->getClass()));
    ret.set("function",  String(bpi->getFunction()));
    ret.set("text",      String(bpi->site()));
  }
  return ret;
}

Variant c_DebuggerClient::t_getstacktrace() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::getstacktrace);
  return m_client->getStackTrace();
}

int c_DebuggerClient::t_getframe() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::getframe);
  return m_client->getFrame();
}

void c_DebuggerClient::t_printframe(int index) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::printframe);
  m_client->printFrame(index, m_client->getStackTrace()[index]);
}

void c_DebuggerClient::t_addcompletion(CVarRef list) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::addcompletion);
  if (list.isInteger()) {
    m_client->addCompletion((DebuggerClient::AutoComplete)list.toInt64());
  } else {
    Array arr = list.toArray(); // handles string, array and iterators
    std::vector<String> items;
    for (ArrayIter iter(arr); iter; ++iter) {
      items.push_back(iter.second().toString());
    }
    m_client->addCompletion(items);
  }
}

Variant c_DebuggerClient::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DebuggerClient, DebuggerClient::__destruct);
  return null;
}

///////////////////////////////////////////////////////////////////////////////
}
