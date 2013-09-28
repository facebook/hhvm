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

#include "hphp/runtime/debugger/cmd/cmd_interrupt.h"
#include "hphp/runtime/debugger/cmd/cmd_break.h"
#include "hphp/runtime/debugger/cmd/cmd_print.h"
#include "hphp/runtime/base/array-init.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdInterrupt::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  assert(m_interrupt != ExceptionHandler); // Server-side only.
  thrift.write(m_interrupt);
  thrift.write(m_program);
  thrift.write(m_errorMsg);
  thrift.write(m_threadId);
  // Used to be m_pendingJump, but that's been removed. Write false until
  // we rev the protocol.
  thrift.write(false);
  if (m_site) {
    thrift.write(true);
    thrift.write(m_site->getFile());
    thrift.write(m_site->getLine0());
    thrift.write(m_site->getChar0());
    thrift.write(m_site->getLine1());
    thrift.write(m_site->getChar1());
    thrift.write(m_site->getNamespace());
    thrift.write(m_site->getClass());
    thrift.write(m_site->getFunction());
    Variant e = m_site->getError();
    if (e.isNull()) {
      thrift.write("");
    } else if (e.isObject()) {
      thrift.write(e.toObject()->o_getClassName());
    } else {
      String ex(BreakPointInfo::ErrorClassName);
      thrift.write(ex);
    }
    thrift.write(e.toString());
  } else {
    thrift.write(false);
  }
  BreakPointInfo::SendImpl(0, m_matched, thrift);
}

void CmdInterrupt::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_interrupt);
  thrift.read(m_program);
  thrift.read(m_errorMsg);
  thrift.read(m_threadId);
  // Used to be m_pendingJump, but that's been removed. Read a dummy bool until
  // we rev the protocol.
  bool dummy;
  thrift.read(dummy);
  m_bpi = BreakPointInfoPtr(new BreakPointInfo());
  bool site; thrift.read(site);
  if (site) {
    thrift.read(m_bpi->m_file);
    thrift.read(m_bpi->m_line1);
    thrift.read(m_bpi->m_char1);
    thrift.read(m_bpi->m_line2);
    thrift.read(m_bpi->m_char2);
    DFunctionInfoPtr func(new DFunctionInfo());
    thrift.read(func->m_namespace);
    thrift.read(func->m_class);
    thrift.read(func->m_function);
    m_bpi->m_funcs.push_back(func);
    thrift.read(m_bpi->m_exceptionClass);
    thrift.read(m_bpi->m_exceptionObject);
  }
  BreakPointInfo::RecvImpl(0, m_matched, thrift);
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
    case HardBreakPoint:
    case BreakPointReached:
    case ExceptionThrown: {
      assert(m_site);
      if (m_site) {
        return m_site->desc();
      }
      return "Breakpoint reached.";
    }
  }

  assert(false);
  return "";
}

void CmdInterrupt::onClient(DebuggerClient &client) {
  client.setCurrentLocation(m_threadId, m_bpi);
  if (!client.getDebuggerClientSmallStep()) {
    // Adjust line and char if it's not small stepping
    if (m_bpi->m_line1 == m_bpi->m_line2) {
      m_bpi->m_char1 = 1;
      m_bpi->m_char2 = 100;
    }
  }
  client.setMatchedBreakPoints(m_matched);

  switch (m_interrupt) {
    case SessionStarted:
      if (!m_program.empty()) {
        client.info("Program %s loaded. Type '[r]un' or '[c]ontinue' to go.",
                     m_program.c_str());
        m_bpi->m_file = m_program;
      }
      break;
    case SessionEnded:
      if (!m_program.empty()) {
        client.info("Program %s exited normally.", m_program.c_str());
      }
      break;
    case RequestStarted:
      if (!m_program.empty()) {
        client.info("Web request %s started.", m_program.c_str());
      }
      break;
    case RequestEnded:
      if (!m_program.empty()) {
        client.info("Web request %s ended.", m_program.c_str());
      }
      break;
    case PSPEnded:
      if (!m_program.empty()) {
        client.info("Post-Send Processing for %s was ended.",
                     m_program.c_str());
      }
      break;
    case HardBreakPoint:
    case BreakPointReached:
    case ExceptionThrown: {
      bool found = false;
      bool toggled = false;
      BreakPointInfoPtrVec *bps = client.getBreakPoints();
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
          if (m_interrupt == BreakPointReached ||
              m_interrupt == HardBreakPoint) {
            client.info("Breakpoint %d reached %s", bp->index(),
                         m_bpi->site().c_str());
            client.shortCode(m_bpi);
          } else {
            if (m_bpi->m_exceptionClass == BreakPointInfo::ErrorClassName) {
              client.info("Breakpoint %d reached: An error occurred %s",
                           bp->index(), m_bpi->site().c_str());
              client.shortCode(m_bpi);
              client.error("Error Message: %s",
                            m_bpi->m_exceptionObject.c_str());
            } else {
              client.info("Breakpoint %d reached: Throwing %s %s",
                           bp->index(),
                           m_bpi->m_exceptionClass.c_str(),
                           m_bpi->site().c_str());
              client.shortCode(m_bpi);
              if (client.getLogFileHandler()) {
                client.output(m_bpi->m_exceptionObject);
              }
            }
          }
          if (!bpm->m_output.empty()) {
            client.print(bpm->m_output);
          }
        }
      }
      if (toggled) {
        CmdBreak::SendClientBreakpointListToServer(client);
      }
      if (!found) {
        if (m_interrupt == HardBreakPoint) {
          // for HardBreakPoint, default the frame to the caller
          client.setFrame(1);
        }
        client.info("Break %s", m_bpi->site().c_str());
        client.shortCode(m_bpi);
      }
      break;
    }
  }

  if (!m_errorMsg.empty()) {
    client.error(m_errorMsg);
  }

  // watches
  switch (m_interrupt) {
    case SessionStarted:
    case RequestStarted:
      break;
    default: {
      DebuggerClient::WatchPtrVec &watches = client.getWatches();
      for (int i = 0; i < (int)watches.size(); i++) {
        if (i > 0) client.output("%s", "");
        client.info("Watch %d: %s =", i + 1, watches[i]->second.c_str());
        Variant v = CmdPrint().processWatch(client, watches[i]->first,
                                            watches[i]->second);
        client.output(CmdPrint::FormatResult(watches[i]->first, v));
      }
    }
  }
}

bool CmdInterrupt::onServer(DebuggerProxy &proxy) {
  return proxy.sendToClient(this);
}

bool CmdInterrupt::shouldBreak(DebuggerProxy &proxy,
                               const BreakPointInfoPtrVec &bps,
                               int stackDepth) {

  switch (m_interrupt) {
    case SessionStarted:
    case SessionEnded:
    case HardBreakPoint:
      return true; // always break
    case RequestStarted:
    case RequestEnded:
    case PSPEnded:
    case BreakPointReached:
    case ExceptionThrown:
      m_matched.clear();
      if (m_site) {
        auto offset = m_site->getCurOffset();
        for (unsigned int i = 0; i < bps.size(); i++) {
          if (bps[i]->m_state != BreakPointInfo::Disabled &&
              bps[i]->breakable(stackDepth, offset) &&
              bps[i]->cmatch(proxy, getInterruptType(), *m_site)) {
            BreakPointInfoPtr bp(new BreakPointInfo());
            *bp = *bps[i]; // make a copy
            m_matched.push_back(bp);
          }
        }
      }
      return !m_matched.empty();
    case ExceptionHandler:
      return false; // For flow control only at this time.
  }
  assert(false);
  return false;
}

std::string CmdInterrupt::getFileLine() const {
  string ret;
  if (m_site) {
    if (m_site->getFile()) {
      ret = m_site->getFile();
    }
    ret += ":" + lexical_cast<string>(m_site->getLine0());
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}}
