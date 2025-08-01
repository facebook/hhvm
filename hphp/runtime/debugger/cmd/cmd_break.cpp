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

#include "hphp/runtime/debugger/cmd/cmd_break.h"

#include "hphp/runtime/debugger/debugger_client.h"

namespace HPHP::Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger)

namespace {
///////////////////////////////////////////////////////////////////////////////

/*
 * Check if the last command parsed by the client has an argument to change the
 * status of a breakpoint.  It is always in the first argument position.
 */

bool hasEnableArg(const DebuggerClient& client) {
  return client.arg(1, "enable");
}

bool hasDisableArg(const DebuggerClient& client) {
  return client.arg(1, "disable");
}

bool hasClearArg(const DebuggerClient& client) {
  return client.arg(1, "clear");
}

bool hasToggleArg(const DebuggerClient& client) {
  return client.arg(1, "toggle");
}

/*
 * Returns true if the last command parsed by the client has an argument that
 * changes the status of a breakpoint.  i.e. clear, enable, disable, or toggle.
 */
bool hasStatusChangeArg(const DebuggerClient& client) {
  return
    hasClearArg(client) || hasEnableArg(client) ||
    hasDisableArg(client) || hasToggleArg(client);
}

///////////////////////////////////////////////////////////////////////////////
}

// Serializes this command into the given Thrift buffer.
void CmdBreak::sendImpl(DebuggerThriftBuffer& thrift) {
  // m_breakpoints is initially set to the breakpoints collection of the
  // client (in validate, which indirectly calls sendImpl). When received
  // via Thrift, m_breakpoints points to a copy that is placed in m_bps.
  assertx(m_breakpoints);
  DebuggerCommand::sendImpl(thrift);
  BreakPointInfo::SendImpl(this->m_version, *m_breakpoints, thrift);
}

// Deserializes a CmdBreak from the given Thrift buffer.
void CmdBreak::recvImpl(DebuggerThriftBuffer& thrift) {
  DebuggerCommand::recvImpl(thrift);
  BreakPointInfo::RecvImpl(this->m_version, m_bps, thrift);
  m_breakpoints = &m_bps;
  // Old senders will set version to 0. A new sender sets it to 1
  // and then expects an answer using version 2.
  // Note that version 1 is the same format as version 0, so old
  // receivers will not break when receiving a version 1 message.
  // This code ensures that version 2 messages are received only
  // by receivers that previously sent a version 1 message (thus
  // indicating their ability to deal with version 2 messages).
  if (this->m_version == 1) this->m_version = 2;
}

// Informs the client of all strings that may follow a break command.
// Used for auto completion. The client uses the prefix of the argument
// following the command to narrow down the list displayed to the user.
void CmdBreak::list(DebuggerClient& client) {
  if (client.argCount() == 0 ||
      (client.argCount() == 1 &&
       (client.arg(1, "regex") || client.arg(1, "once")))) {
    static const char *keywords1[] = {
      "regex",
      "once",
      "start",
      "end",
      "psp",
      "list",
      "clear",
      "toggle",
      "enable",
      "disable",
      nullptr
    };
    client.addCompletion(keywords1);
    client.addCompletion(DebuggerClient::AutoCompleteFileNames);
    client.addCompletion(DebuggerClient::AutoCompleteFunctions);
    client.addCompletion(DebuggerClient::AutoCompleteClasses);
    client.addCompletion(DebuggerClient::AutoCompleteClassMethods);
  } else if (client.argCount() == 1) {
    if (hasStatusChangeArg(client)) {
      client.addCompletion("all");
    } else if (!client.arg(1, "list")) {
      client.addCompletion(DebuggerClient::AutoCompleteFileNames);
    }
  } else {
    client.addCompletion(DebuggerClient::AutoCompleteCode);
  }
}

// The text to display when the debugger client processes "help break".
void CmdBreak::help(DebuggerClient& client) {
  client.helpTitle("Breaking in HPHPD is no longer supported");
  client.helpSection(
    "We do not support breakpoints on command line. 'b' and 'hphpd_break()' will no"
    "longer set a breakpoint. Please use the VSCode Debugger extension instead for debugging:"
    "https://www.internalfb.com/wiki/VSCode_Debugger/"
  );

  client.help("%s", "");
}

// Carries out the "break list" command.
void CmdBreak::processList(DebuggerClient &client) {
  m_breakpoints = client.getBreakPoints();
  updateServer(client);
  for (auto& bpi : *m_breakpoints) {
    auto bound = bpi->m_bindState != BreakPointInfo::Unknown;
    if (!bound && !client.isLocal() &&
        (bpi->m_interruptType == RequestStarted ||
        bpi->m_interruptType == RequestEnded ||
        bpi->m_interruptType == PSPEnded)) {
      bound = true;
    }
    auto const boundStr = bound ? "" : " (unbound)";
    client.print("  %d\t%s  %s%s", bpi->index(), bpi->state(true).c_str(),
                  bpi->desc().c_str(), boundStr);
  }
  if (m_breakpoints->empty()) {
    client.tutorial(
      "Use '[b]reak ?|[h]elp' to read how to set breakpoints. "
    );
  } else {
    client.tutorial(
      "Use '[b]reak [c]lear {index}|[a]ll' to remove breakpoint(s). "
      "Use '[b]reak [t]oggle {index}|[a]ll' to change their states."
    );
  }
}

// Carries out commands that change the status of a breakpoint.
void CmdBreak::processStatusChange(DebuggerClient& client) {
  m_breakpoints = client.getBreakPoints();
  if (m_breakpoints->empty()) {
    client.error("There is no breakpoint to clear or toggle.");
    client.tutorial(
      "Use '[b]reak ?|[h]elp' to read how to set breakpoints. "
    );
    return;
  }

  if (client.argCount() == 1) {
    auto matched = client.getMatchedBreakPoints();
    auto bps = client.getBreakPoints();
    bool found = false;
    for (auto& bpm : *matched) {
      BreakPointInfoPtr bp;
      size_t index = 0;
      for (; index < bps->size(); ++index) {
        if (bpm->same((*bps)[index])) {
          bp = (*bps)[index];
          break;
        }
      }
      if (bp) {
        const char *action;
        if (hasClearArg(client)) {
          action = "cleared";
          bps->erase(bps->begin() + index);
        } else if (hasEnableArg(client)) {
          action = "updated";
          bp->setState(BreakPointInfo::Always);
        } else if (hasDisableArg(client)) {
          action = "updated";
          bp->setState(BreakPointInfo::Disabled);
        } else {
          assertx(hasToggleArg(client));
          action = "updated";
          bp->toggle();
        }
        client.info("Breakpoint %d is %s %s", bp->index(),
                     action, bp->site().c_str());
        found = true;
      }
    }
    if (found) {
      updateServer(client);
      return;
    }

    client.error("There is no current breakpoint to clear or toggle.");
    return;
  }

  if (client.arg(2, "all")) {
    if (hasClearArg(client)) {
      while (m_breakpoints->size() > 0) {
        m_breakpoints->erase(m_breakpoints->end() - 1);
      }
      updateServer(client);
      client.info("All breakpoints are cleared.");
      return;
    }

    for (auto& bpi : *m_breakpoints) {
      if (hasEnableArg(client)) {
        bpi->setState(BreakPointInfo::Always);
      }
      else if (hasDisableArg(client)) {
        bpi->setState(BreakPointInfo::Disabled);
      }
      else {
        assertx(hasToggleArg(client));
        bpi->toggle();
      }
    }

    updateServer(client);
    return processList(client);
  }

  std::string snum = client.argValue(2);
  if (!DebuggerClient::IsValidNumber(snum)) {
    client.error("'[b]reak [c]lear|[t]oggle' needs an {index} argument.");
    client.tutorial(
      "You will have to run '[b]reak [l]ist' first to see a list of valid "
      "numbers or indices to specify."
    );
    return;
  }

  int index = -1;
  int num = atoi(snum.c_str());
  for (size_t i = 0; i < m_breakpoints->size(); ++i) {
    if (m_breakpoints->at(i)->index() == num) {
      index = i;
      break;
    }
  }
  if (index < 0) {
    client.error("\"%s\" is not a valid breakpoint index. Choose one from "
                  "this list:", snum.c_str());
    processList(client);
    return;
  }

  auto bpi = (*m_breakpoints)[index];
  if (hasClearArg(client)) {
    m_breakpoints->erase(m_breakpoints->begin() + index);
    updateServer(client);
    client.info("Breakpoint %d cleared %s", bpi->index(),
                 bpi->desc().c_str());
  } else if (hasEnableArg(client)) {
    bpi->setState(BreakPointInfo::Always);
    updateServer(client);
    client.info("Breakpoint %d's state is changed to %s.", bpi->index(),
                 bpi->state(false).c_str());
  } else if (hasDisableArg(client)) {
    bpi->setState(BreakPointInfo::Disabled);
    updateServer(client);
    client.info("Breakpoint %d's state is changed to %s.", bpi->index(),
                 bpi->state(false).c_str());
  } else {
    assertx(hasToggleArg(client));
    bpi->toggle();
    updateServer(client);
    client.info("Breakpoint %d's state is changed to %s.", bpi->index(),
                 bpi->state(false).c_str());
  }
}

/*
 * Uses the client to send this command to the server, which will update its
 * breakpoint list with the one in this command.  The client will block until
 * the server echoes this command back to it.  The echoed command is discarded.
 * If the server checked the validity of the breakpoints, the values of the
 * m_bindState flags are copied to the client's breakpoint list.
 */
void CmdBreak::updateServer(DebuggerClient& client) {
  m_body = "update";
  auto serverReply = client.xend<CmdBreak>(this);
  if (serverReply->m_version == 2) {
    // The server will have checked the breakpoint list for validity.  Transfer
    // the results to the local breakpoint list.
    auto& cbreakpoints = *client.getBreakPoints();
    auto& sbreakpoints = *serverReply->m_breakpoints;
    auto const csize = cbreakpoints.size();
    assertx(csize == sbreakpoints.size());
    for (size_t i = 0; i < csize; ++i) {
      cbreakpoints[i]->m_bindState = sbreakpoints[i]->m_bindState;
    }
  }
}

// Creates a new CmdBreak instance, sets its breakpoints to the client's
// list, sends the command to the server and waits for a response.
void CmdBreak::SendClientBreakpointListToServer(DebuggerClient& client) {
  auto cmd = CmdBreak();
  cmd.m_breakpoints = client.getBreakPoints();
  cmd.updateServer(client);
}

void ReportBreakpointBindState(DebuggerClient& client, BreakPointInfoPtr bpi) {
  switch (bpi->m_bindState) {
  case BreakPointInfo::KnownToBeValid:
    client.info("Breakpoint %d set %s", bpi->index(), bpi->desc().c_str());
    break;
  case BreakPointInfo::KnownToBeInvalid:
    client.info("Breakpoint %d not set %s", bpi->index(), bpi->desc().c_str());
    if (!bpi->getClass().empty()) {
      client.info("Because method %s does not exist.",
                  bpi->getFuncName().c_str());
    } else if (!bpi->getExceptionClass().empty()) {
      client.info("Because class %s is not an exception.",
                  bpi->getExceptionClass().c_str());
    } else {
      client.info("Because the line does not exist or is not executable code.");
    }
    break;
  case BreakPointInfo::Unknown:
    client.info("Breakpoint %d set %s", bpi->index(), bpi->desc().c_str());
    if (!bpi->getClass().empty()) {
      client.info("But won't break until class %s has been loaded.",
                  bpi->getClass().c_str());
    } else if (!bpi->getFuncName().empty()) {
      client.info("But won't break until function %s has been loaded.",
                  bpi->getFuncName().c_str());
    } else if (!bpi->getExceptionClass().empty()) {
      client.info("But note that class %s has yet been loaded.",
                  bpi->getExceptionClass().c_str());
    } else if (bpi->m_interruptType == RequestStarted ||
        bpi->m_interruptType == RequestEnded ||
        bpi->m_interruptType == PSPEnded) {
      if (client.isLocal()) {
        client.info("But won't break until connected to a server.");
      }
    } else {
      client.info("But won't break until file %s has been loaded.",
                  bpi->m_file.c_str());
    }
    break;
  }
}

/*
 * Adds conditional or watch clause to the breakpoint info if needed.  Then adds
 * the breakpoint to client's list and sends this command to the server so that
 * it too can update it's list.  Returns false if the breakpoint is not well
 * formed.
 */
bool CmdBreak::addToBreakpointListAndUpdateServer(
  DebuggerClient& client,
  BreakPointInfoPtr bpi,
  int index
) {
  ++index;
  if (client.arg(index, "if")) {
    bpi->setClause(client.lineRest(++index), true);
  } else if (client.arg(index, "&&")) {
    bpi->setClause(client.lineRest(++index), false);
  }

  if (bpi->valid()) {
    m_breakpoints = client.getBreakPoints();

    for (auto& set_bpi : *m_breakpoints) {
      if (set_bpi->same(bpi)) {
        // If the user is trying to create a breakpoint where a disabled
        // breakpoint already exists, then re-enable the original breakpoint.
        if (set_bpi->m_state == BreakPointInfo::Disabled) {
          set_bpi->setState(BreakPointInfo::Always);
          client.info(
            "Breakpoint %" PRId16 " is re-enabled %s",
            set_bpi->m_index,
            set_bpi->site().c_str()
          );
          updateServer(client);
          return true;
        }

        client.error("Breakpoint was already set previously.");
        return true;
      }
    }

    m_breakpoints->push_back(bpi);
    updateServer(client);
    ReportBreakpointBindState(client, bpi);
    if (bpi->m_bindState == BreakPointInfo::KnownToBeInvalid) {
      m_breakpoints->pop_back();
    }
    return true;
  }

  // TODO: Keep more detailed error information in the BreakPointInfo and emit
  // that information here.  Well, first factor out this code into a separate
  // check and report method and report from there.  See task 2368334.
  if (!bpi->m_url.empty()) {
    client.error("@{url} cannot be specified alone.");
  } else {
    client.error("Breakpoint was not set in right format.");
  }
  return false;
}

// Carries out the Break command. This always involves an action on the
// client and usually, but not always, involves the server by sending
// this command to the server and waiting for its response.
void CmdBreak::onClient(DebuggerClient& client) {
  client.error("Breakpoints are no longer supported on hphpd. Please use the VSCode Debugger instead: https://www.internalfb.com/wiki/VSCode_Debugger/");
}

/*
 * Updates the breakpoint list in the proxy with the new list received from the
 * client.  Then sends the command back to the client as confirmation.  Returns
 * false if the confirmation message send failed.
 */
bool CmdBreak::onServer(DebuggerProxy& proxy) {
  if (m_body == "update") {
    proxy.setBreakPoints(m_bps);
    m_breakpoints = &m_bps;
    return proxy.sendToClient(this);
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
