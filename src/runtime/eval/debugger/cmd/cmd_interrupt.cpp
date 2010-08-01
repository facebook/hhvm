/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/eval/debugger/cmd/cmd_interrupt.h>
#include <runtime/eval/debugger/cmd/cmd_break.h>
#include <runtime/eval/debugger/cmd/cmd_print.h>

using namespace std;
using namespace boost;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdInterrupt::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_interrupt);
  thrift.write(m_program);
  thrift.write(m_threadId);
  thrift.write(m_pendingJump);
  if (m_site) {
    thrift.write(true);
    thrift.write(m_site->getFile());
    thrift.write(m_site->getLine());
    thrift.write(m_site->getNamespace());
    thrift.write(m_site->getClass());
    thrift.write(m_site->getFunction());
    Object e = m_site->getException();
    if (e.isNull()) {
      thrift.write("");
    } else {
      thrift.write(e->o_getClassName());
    }
    thrift.write(DebuggerClient::FormatVariable(e));
  } else {
    thrift.write(false);
  }
  BreakPointInfo::SendImpl(m_matched, thrift);
}

void CmdInterrupt::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_interrupt);
  thrift.read(m_program);
  thrift.read(m_threadId);
  thrift.read(m_pendingJump);
  m_bpi = BreakPointInfoPtr(new BreakPointInfo());
  bool site; thrift.read(site);
  if (site) {
    thrift.read(m_bpi->m_file);
    thrift.read(m_bpi->m_line1);
    DFunctionInfoPtr func(new DFunctionInfo());
    thrift.read(func->m_namespace);
    thrift.read(func->m_class);
    thrift.read(func->m_function);
    m_bpi->m_funcs.push_back(func);
    thrift.read(m_bpi->m_exceptionClass);
    thrift.read(m_bpi->m_exceptionObject);
  }
  BreakPointInfo::RecvImpl(m_matched, thrift);
}

std::string CmdInterrupt::desc() const {
  switch (m_interrupt) {
    case SessionStarted:
      if (!m_program.empty()) {
        return m_program + " loaded.";
      }
      return "Program loaded.";
    case SessionEnded:
      if (!m_program.empty()) {
        return m_program + " exited normally.";
      }
      return "Program exited normally.";
    case RequestStarted:
      if (!m_program.empty()) {
        return m_program + " started.";
      }
      return "Web request started.";
    case RequestEnded:
      if (!m_program.empty()) {
        return m_program + " ended.";
      }
      return "Web request ended.";
    case PSPEnded:
      if (!m_program.empty()) {
        return "Post-Send Processing for " + m_program + " was ended.";
      }
      return "Post-Send Processing was ended.";
    case BreakPointReached:
    case ExceptionThrown: {
      if (m_bpi) {
        if (m_interrupt == BreakPointReached) {
          return m_bpi->site();
        }
        return "Throwing " + m_bpi->m_exceptionClass + " " + m_bpi->site();
      }
      return "Breakpoint reached.";
    }
  }

  ASSERT(false);
  return "";
}

bool CmdInterrupt::onClient(DebuggerClient *client) {
  client->setCurrentLocation(m_threadId, m_bpi);
  client->setMatchedBreakPoints(m_matched);

  switch (m_interrupt) {
    case SessionEnded:
    case RequestEnded:
    case PSPEnded:
      if (m_pendingJump) {
        client->error("Your jump point cannot be reached. You may only jump "
                      "to certain parallel or outer execution points.");
      }
      break;
  }

  switch (m_interrupt) {
    case SessionStarted:
      if (!m_program.empty()) {
        client->info("Program %s loaded. Type '[r]un' or '[c]ontinue' to go.",
                     m_program.c_str());
        m_bpi->m_file = m_program;
      }
      break;
    case SessionEnded:
      if (!m_program.empty()) {
        client->info("Program %s exited normally.", m_program.c_str());
      }
      break;
    case RequestStarted:
      if (!m_program.empty()) {
        client->info("Web request %s started.", m_program.c_str());
      }
      break;
    case RequestEnded:
      if (!m_program.empty()) {
        client->info("Web request %s ended.", m_program.c_str());
      }
      break;
    case PSPEnded:
      if (!m_program.empty()) {
        client->info("Post-Send Processing for %s was ended.",
                     m_program.c_str());
      }
      break;
    case BreakPointReached:
    case ExceptionThrown: {
      bool found = false;
      bool toggled = false;
      BreakPointInfoPtrVec *bps = client->getBreakPoints();
      for (unsigned int i = 0; i < m_matched.size(); i++) {
        BreakPointInfoPtr bpm = m_matched[i];
        BreakPointInfoPtr bp;
        int index = 0;
        for (; index < (int)bps->size(); index++) {
          if (bpm->same((*bps)[index])) {
            bp = (*bps)[index];
            break;
          }
        }
        if (bp) {
          found = true;
          if (bp->m_state == BreakPointInfo::Once) {
            bp->m_state = BreakPointInfo::Disabled;
            toggled = true;
          }
          if (m_interrupt == BreakPointReached) {
            client->info("Breakpoint %d reached %s", index + 1,
                         m_bpi->site().c_str());
          } else {
            client->info("Breakpoint %d reached: Throwing %s %s", index + 1,
                         m_bpi->m_exceptionClass.c_str(),
                         m_bpi->site().c_str());
            client->output(m_bpi->m_exceptionObject);
          }
          if (!bpm->m_output.empty()) {
            client->print(bpm->m_output);
          }
        }
      }
      if (toggled) {
        CmdBreak().update(client);
      }
      if (!found) {
        client->info("Break %s", m_bpi->site().c_str());
      }
      break;
    }
  }

  // watches
  switch (m_interrupt) {
    case SessionStarted:
    case RequestStarted:
      break;
    default: {
      DebuggerClient::WatchPtrVec &watches = client->getWatches();
      for (int i = 0; i < (int)watches.size(); i++) {
        if (i > 0) client->output("");
        client->info("Watch %d: %s =", i, watches[i]->second.c_str());
        CmdPrint().processWatch(client, watches[i]->first, watches[i]->second);
      }
    }
  }

  return true;
}

bool CmdInterrupt::onServer(DebuggerProxy *proxy) {
  return proxy->send(this);
}

bool CmdInterrupt::shouldBreak(const BreakPointInfoPtrVec &bps) {
  switch (m_interrupt) {
    case SessionStarted:
    case SessionEnded:
      return true; // always break
    case RequestStarted:
    case RequestEnded:
    case PSPEnded:
    case BreakPointReached:
    case ExceptionThrown:
      m_matched.clear();
      if (m_site) {
        for (unsigned int i = 0; i < bps.size(); i++) {
          if (bps[i]->m_state != BreakPointInfo::Disabled &&
              bps[i]->match((InterruptType)m_interrupt, *m_site)) {
            BreakPointInfoPtr bp(new BreakPointInfo());
            *bp = *bps[i]; // make a copy
            m_matched.push_back(bp);
          }
        }
      }
      return !m_matched.empty();
  }
  ASSERT(false);
  return false;
}

std::string CmdInterrupt::getFileLine() const {
  string ret;
  if (m_site) {
    if (m_site->getFile()) {
      ret = m_site->getFile();
    }
    ret += ":" + lexical_cast<string>(m_site->getLine());
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}}
