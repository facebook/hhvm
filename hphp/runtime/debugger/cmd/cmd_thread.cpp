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

#include "hphp/runtime/debugger/cmd/cmd_thread.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/util/process.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdThread::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_out);
  thrift.write(m_threads);
}

void CmdThread::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_out);
  thrift.read(m_threads);
}

void CmdThread::list(DebuggerClient &client) {
  if (client.argCount() == 0) {
    static const char *keywords[] =
      { "list", "normal", "sticky", "exclusive", nullptr };
    client.addCompletion(keywords);
  }
}

void CmdThread::help(DebuggerClient &client) {
  client.helpTitle("Thread Command");
  client.helpCmds(
    "[t]hread",                 "displays current thread's information",
    "[t]hread [l]ist",          "lists all threads at break",
    "[t]hread {index}",         "switches to the specified thread",
    "[t]hread [n]ormal",        "breaks all threads",
    "[t]hread [s]ticky",        "only send command to current thread",
    "[t]hread [e]xclusive",     "only break current thread",
    nullptr
  );
  client.helpBody(
    "Use '[t]hread' alone to display information of current thread.\n"
    "\n"
    "When a thread is at break, you may specify how other threads should "
    "behave if they also happen to hit some breakpoints. Normally, other "
    "threads will also break, and they will interrupt debugger session "
    "with their breakpoints. So breaks from different threads may interleave. "
    "If '[t]hread [s]ticky' is specified, all other threads will wait until "
    "current thread is finished. This will help debugging to focus on just "
    "one thread without losing breaks from other threads. If there is no need "
    "to hold up any other threads, use '[t]hread [e]xclusive'. Then other "
    "threads will not break at all. This mode is useful for live debugging "
    "a production server, without interrupting many threads at a time. Use "
    "'[t]hread [n]ormal' to change thread mode back to normal.\n"
    "\n"
    "Some debugging commands will automatically turn thread mode to sticky. "
    "These include continue, step, next or out commands with a counter of "
    "more than 1. These commands imply non-interruption from another thread. "
    "The mode will remain even after these commands until '[t]hread [n]ormal' "
    "is issued."
    "\n"
    "When multple threads hit breakpoints at the same time, use '[t]hread "
    "[l]ist' command to display their indices, which can be used to switch "
    "between them with '[t]hread {index}'."
  );
}

void CmdThread::processList(DebuggerClient &client, bool output /* = true */) {
  m_body = "list";
  CmdThreadPtr res = client.xend<CmdThread>(this);
  client.updateThreads(res->m_threads);
  if (!output) return;

  for (int i = 0; i < (int)res->m_threads.size(); i++) {
    DThreadInfoPtr thread = res->m_threads[i];
    const char *flag = " ";
    if (thread->m_id == client.getCurrentThreadId()) {
      flag = "*";
    }
    client.print("%4d %s %s (%" PRId64 ") %s\n     %s", thread->m_index,
                  flag, thread->m_type.c_str(), thread->m_id,
                  thread->m_url.c_str(), thread->m_desc.c_str());
  }
}

void CmdThread::onClient(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;
  if (client.argCount() > 1) {
    help(client);
    return;
  }

  if (client.argCount() == 0) {
    m_body = "info";
    CmdThreadPtr res = client.xend<CmdThread>(this);
    client.print(res->m_out);
  } else if (client.arg(1, "list")) {
    processList(client);
  } else if (client.arg(1, "normal")) {
    m_body = "normal";
    client.sendToServer(this);
    client.info("Thread is running in normal mode now. Other threads will "
                 "interleave when they hit breakpoints as well.");
  } else if (client.arg(1, "sticky")) {
    m_body = "sticky";
    client.sendToServer(this);
    client.info("Thread is running in sticky mode now. All other threads "
                 "will wait until this thread finishes, when they hit "
                 "breakpoints.");
  } else if (client.arg(1, "exclusive")) {
    m_body = "exclusive";
    client.sendToServer(this);
    client.info("Thread is running in exclusive mode now. All other threads "
                 "will not break, even when they hit breakpoints.");
  } else {
    string snum = client.argValue(1);
    if (!DebuggerClient::IsValidNumber(snum)) {
      client.error("'[t]hread {index}' needs a numeric argument.");
      client.tutorial(
        "You will have to run '[t]hread [l]ist' first to see a list of valid "
        "numbers or indices to specify. Thread 1 is always your current "
        "thread. If that's the only thread on the list, you do not have "
        "another thread at break to switch to."
      );
      return;
    }

    int num = atoi(snum.c_str());
    DThreadInfoPtr thread = client.getThread(num);
    if (!thread) {
      processList(client, false);
      thread = client.getThread(num);
      if (!thread) {
        client.error("\"%s\" is not a valid thread index. Choose one from "
                      "this list:", snum.c_str());
        processList(client);
        return;
      }
    }

    if (thread->m_id == client.getCurrentThreadId()) {
      client.info("This is your current thread already.");
      return;
    }

    m_body = "switch";
    m_threads.push_back(thread);
    client.sendToServer(this);
    throw DebuggerConsoleExitException();
  }
}

void CmdThread::debuggerInfo(InfoVec &info) {
  Add(info, "Host",       Process::GetHostName());
  Add(info, "Binary",     Process::GetAppName());
  Add(info, "Version",    Process::GetAppVersion());
  Add(info, "Process ID", FormatNumber("%lld", Process::GetProcessId()));
  Add(info, "Thread ID",  FormatNumber("0x%llx", (int64_t)Process::GetThreadId()));
}

bool CmdThread::onServer(DebuggerProxy &proxy) {
  if (m_body == "info") {
    // collect info
    InfoVec info;
    debuggerInfo(info);
    Transport *transport = g_context->getTransport();
    if (transport) {
      transport->debuggerInfo(info);
    } else {
      Add(info, "Thread Type", proxy.getThreadType());
    }
    g_context->debuggerInfo(info);

    m_out = DebuggerClient::FormatInfoVec(info);
    return proxy.sendToClient(this);
  }

  if (m_body == "list") {
    proxy.getThreads(m_threads);
    return proxy.sendToClient(this);
  }
  if (m_body == "switch") {
    if (!m_threads.empty()) {
      proxy.switchThread(m_threads[0]);
      m_exitInterrupt = true;
      return true;
    }
  }

  if (m_body == "normal") {
    proxy.switchThreadMode(DebuggerProxy::Normal);
    return true;
  }
  if (m_body == "sticky") {
    proxy.switchThreadMode(DebuggerProxy::Sticky);
    return true;
  }
  if (m_body == "exclusive") {
    proxy.switchThreadMode(DebuggerProxy::Exclusive);
    return true;
  }

  assert(false);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}}
