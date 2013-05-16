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

#include "hphp/runtime/eval/debugger/cmd/cmd_break.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

// Serializes this command into the given Thrift buffer.
void CmdBreak::sendImpl(DebuggerThriftBuffer &thrift) {
  // m_breakpoints is initially set to the breakpoints collection of the
  // client (in validate, which indirectly calls sendImpl). When received
  // via Thrift, m_breakpoints points to a copy that is placed in m_bps.
  assert(m_breakpoints);
  DebuggerCommand::sendImpl(thrift);
  BreakPointInfo::SendImpl(*m_breakpoints, thrift);
}

// Deserializes a CmdBreak from the given Thrift buffer.
void CmdBreak::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  BreakPointInfo::RecvImpl(m_bps, thrift);
}

// Informs the client of all strings that may follow a break command.
// Used for auto completion. The client uses the prefix of the argument
// following the command to narrow down the list displayed to the user.
void CmdBreak::list(DebuggerClient *client) {
  if (client->argCount() == 0 ||
      (client->argCount() == 1 &&
       (client->arg(1, "regex") || client->arg(1, "once")))) {
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
    client->addCompletion(keywords1);
    client->addCompletion(DebuggerClient::AutoCompleteFileNames);
    client->addCompletion(DebuggerClient::AutoCompleteFunctions);
    client->addCompletion(DebuggerClient::AutoCompleteClasses);
    client->addCompletion(DebuggerClient::AutoCompleteClassMethods);
  } else if (client->argCount() == 1) {
    if (hasStatusChangeArg(client)) {
      client->addCompletion("all");
    } else if (!client->arg(1, "list")) {
      client->addCompletion(DebuggerClient::AutoCompleteFileNames);
    }
  } else {
    client->addCompletion(DebuggerClient::AutoCompleteCode);
  }
}

// The text to display when the debugger client processes "help break".
bool CmdBreak::help(DebuggerClient *client) {
  client->helpTitle("Break Command");
  client->helpCmds(
    "[b]reak",                  "breaks at current line of code",
    "[b]reak {exp}",            "breaks at matching location",
    "",                         "",
    "[b]reak [o]nce  {above}",  "breaks just once then disables it",
    "",                         "",
    "[b]reak [s]tart {url}",    "breaks at start of web request",
    "[b]reak [e]nd {url}",      "breaks at end of web request",
    "[b]reak [p]sp {url}",      "breaks at end of psp",
    "",                         "",
    "[b]reak {above} if {php}", "breaks if condition meets",
    "[b]reak {above} && {php}", "breaks and evaluates an expression",
    "",                         "",
    "[b]reak [l]ist",           "lists all breakpoints",
    "[b]reak [c]lear {index}",  "clears the n-th breakpoint on list",
    "[b]reak [c]lear [a]ll",    "clears all breakpoints",
    "[b]reak [c]lear",          "clears current breakpoint",
    "[b]reak [t]oggle {index}", "toggles the n-th breakpoint on list",
    "[b]reak [t]oggle [a]ll",   "toggles all breakpoints",
    "[b]reak [t]oggle",         "toggles current breakpoint",
    "[b]reak [e]nable {index}", "enables the n-th breakpoint on list",
    "[b]reak [e]nable [a]ll",   "enables all breakpoints",
    "[b]reak [e]nable",         "enables current breakpoint",
    "[b]reak [d]isable {index}","disables the n-th breakpoint on list",
    "[b]reak [d]isable [a]ll",  "disables all breakpoints",
    "[b]reak [d]isable",        "disables current breakpoint",
    nullptr
  );

  client->helpTitle("Where to break?");
  client->helpSection(
    "There are many ways to specify a source file location to set a "
    "breakpoint, but it's ONE single string without whitespaces. The "
    "format looks like this,\n"
    "\n"
    "\tfile location: {file}:{line}\n"
    "\tfunction call: {func}()\n"
    "\tmethod invoke: {cls}::{method}()\n"
    "\n"
    "For examples,\n"
    "\n"
    "\tb mypage.php:123\n"
    "\tb foo()\n"
    "\tb MyClass::foo()\n"
  );

  client->helpTitle("Special Breakpoints");
  client->helpSection(
    "There are special breakpoints what can only be set by names:\n"
    "\n"
    "\tstart\n"
    "\tend\n"
    "\tpsp\n"
    "\n"
    "They represent different time points of a web request. 'start' is at the "
    "beginning of a web request, when no PHP file is invoked yet, but query "
    "string and server variables are already prepared. 'end' is at the end of "
    "a web request, but BEFORE post-send processing (psp). 'psp' is at END of "
    "psp, not beginning. To set a breakpoint at the beginning of psp, use "
    "'end', because end of a request is the same as beginning of psp."
  );

  client->helpTitle("Conditional Breakpoints and Watchpoints");
  client->helpSection(
    "Every breakpoint can specify a condition, which is an arbitrary PHP "
    "expression that will be evaluated to TRUE or FALSE. When TRUE, it will "
    "break. When FALSE, it will continue without break. \"&&\" is similar to "
    "\"if\", except it will always break, regardless what the expression "
    "returns. This is useful to watch variables at breakpoints. For example,\n"
    "\n"
    "\tb mypage.php:123 && print $a\n"
    "\n"
    "So every time it breaks at mypage.php line 123, it will print out $a."
  );

  client->helpTitle("Breakpoint States and List");
  client->helpSection(
    "Every breakpoint has 3 states: ALWAYS, ONCE, DISABLED. Without keyword "
    "\"once\", a breakpoint is in ALWAYS state. ONCE breakpoints will turn "
    "into DISABLED after it's hit once. DISABLED breakpoints will not break, "
    "but they are kept in the list, so one can run 'b l' command and 'b t' "
    "command to toggle their states.\n"
    "\n"
    "Use '[b]reak [l]ist' command to view indices of different breakpoints. "
    "Then use those indices to clear or toggle their states. This list of "
    "breakpoints and their states will remain the same when switching to "
    "different machines, sandboxes and threads."
  );

  client->helpTitle("Hard Breakpoints");
  client->helpSection(
    "From within PHP code, you can place a function call hphpd_break() to "
    "embed a breakpoint. You may also specify a condition as the function's "
    "parameter, so it breaks when the condition is met. Please read about "
    "this function for more details with '[i]nfo hphpd_break'."
  );

  client->help("");
  return true;
}

// Carries out the "break list" command.
bool CmdBreak::processList(DebuggerClient *client) {
  m_breakpoints = client->getBreakPoints();
  for (int i = 0; i < (int)m_breakpoints->size(); i++) {
    BreakPointInfoPtr bpi = m_breakpoints->at(i);
    client->print("  %d\t%s  %s", bpi->index(), bpi->state(true).c_str(),
                  bpi->desc().c_str());
  }
  if (m_breakpoints->empty()) {
    client->tutorial(
      "Use '[b]reak ?|[h]elp' to read how to set breakpoints. "
    );
  } else {
    client->tutorial(
      "Use '[b]reak [c]lear {index}|[a]ll' to remove breakpoint(s). "
      "Use '[b]reak [t]oggle {index}|[a]ll' to change their states."
    );
  }
  return true;
}

// Carries out commands that change the status of a breakpoint.
bool CmdBreak::processStatusChange(DebuggerClient *client) {
  m_breakpoints = client->getBreakPoints();
  if (m_breakpoints->empty()) {
    client->error("There is no breakpoint to clear or toggle.");
    client->tutorial(
      "Use '[b]reak ?|[h]elp' to read how to set breakpoints. "
    );
    return true;
  }

  if (client->argCount() == 1) {
    BreakPointInfoPtrVec *matched = client->getMatchedBreakPoints();
    BreakPointInfoPtrVec *bps = client->getBreakPoints();
    bool found = false;
    for (unsigned int i = 0; i < matched->size(); i++) {
      BreakPointInfoPtr bpm = (*matched)[i];
      BreakPointInfoPtr bp;
      int index = 0;
      for (; index < (int)bps->size(); index++) {
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
          assert(hasToggleArg(client));
          action = "updated";
          bp->toggle();
        }
        client->info("Breakpoint %d is %s %s", bp->index(),
                     action, bp->site().c_str());
        found = true;
      }
    }
    if (found) {
      updateServer(client);
      return true;
    }

    client->error("There is no current breakpoint to clear or toggle.");
    return true;
  }

  if (client->arg(2, "all")) {
    if (hasClearArg(client)) {
      m_breakpoints->clear();
      updateServer(client);
      client->info("All breakpoints are cleared.");
      return true;
    }

    for (unsigned int i = 0; i < m_breakpoints->size(); i++) {
      BreakPointInfoPtr bpi = (*m_breakpoints)[i];
      if (hasEnableArg(client)) {
        bpi->setState(BreakPointInfo::Always);
      }
      else if (hasDisableArg(client)) {
        bpi->setState(BreakPointInfo::Disabled);
      }
      else {
        assert(hasToggleArg(client));
        bpi->toggle();
      }
    }

    updateServer(client);
    return processList(client);
  }

  string snum = client->argValue(2);
  if (!DebuggerClient::IsValidNumber(snum)) {
    client->error("'[b]reak [c]lear|[t]oggle' needs an {index} argument.");
    client->tutorial(
      "You will have to run '[b]reak [l]ist' first to see a list of valid "
      "numbers or indices to specify."
    );
    return true;
  }

  int index = -1;
  int num = atoi(snum.c_str());
  for (unsigned int i = 0; i < m_breakpoints->size(); i++) {
    if (m_breakpoints->at(i)->index() == num) {
      index = i;
      break;
    }
  }
  if (index < 0) {
    client->error("\"%s\" is not a valid breakpoint index. Choose one from "
                  "this list:", snum.c_str());
    processList(client);
    return true;
  }

  BreakPointInfoPtr bpi = (*m_breakpoints)[index];
  if (hasClearArg(client)) {
    m_breakpoints->erase(m_breakpoints->begin() + index);
    updateServer(client);
    client->info("Breakpoint %d cleared %s", bpi->index(),
                 bpi->desc().c_str());
  } else if (hasEnableArg(client)) {
    bpi->setState(BreakPointInfo::Always);
    updateServer(client);
    client->info("Breakpoint %d's state is changed to %s.", bpi->index(),
                 bpi->state(false).c_str());
  } else if (hasDisableArg(client)) {
    bpi->setState(BreakPointInfo::Disabled);
    updateServer(client);
    client->info("Breakpoint %d's state is changed to %s.", bpi->index(),
                 bpi->state(false).c_str());
  } else {
    assert(hasToggleArg(client));
    bpi->toggle();
    updateServer(client);
    client->info("Breakpoint %d's state is changed to %s.", bpi->index(),
                 bpi->state(false).c_str());
  }

  return true;
}

// Uses the client to send this command to the server, which
// will update its breakpoint list with the one in this command.
// The client will block until the server echoes
// this command back to it. The echoed command is discarded.
bool CmdBreak::updateServer(DebuggerClient *client) {
  m_body = "update";
  client->xend<CmdBreak>(this);
  return true;
}

// Creates a new CmdBreak instance, sets its breakpoints to the client's
// list, sends the command to the server and waits for a response.
bool CmdBreak::SendClientBreakpointListToServer(DebuggerClient *client) {
  auto cmd = CmdBreak();
  cmd.m_breakpoints = client->getBreakPoints();
  return cmd.updateServer(client);
}

// Adds conditional or watch clause to the breakpoint info if needed.
// Then adds the breakpoint to client's list and sends this command
// to the server so that it too can update it's list.
// Returns false if the breakpoint is not well formed.
bool CmdBreak::addToBreakpointListAndUpdateServer(
    DebuggerClient *client, BreakPointInfoPtr bpi, int index) {
  ++index;
  if (client->arg(index, "if")) {
    bpi->setClause(client->lineRest(++index), true);
  } else if (client->arg(index, "&&")) {
    bpi->setClause(client->lineRest(++index), false);
  }

  if (bpi->valid()) {
    m_breakpoints = client->getBreakPoints();
    for (int i = 0; i < (int)m_breakpoints->size(); i++) {
      if ((*m_breakpoints)[i]->same(bpi)) {
        client->error("Breakpoint was already set previously.");
        return true;
      }
    }
    m_breakpoints->push_back(bpi);
    updateServer(client);
    client->info("Breakpoint %d set %s", bpi->index(), bpi->desc().c_str());
    return true;
  }

  // TODO: keep more detailed error information in the BreakPointInfo
  // and emit that information here. Well, first factor out this code
  // into a separate check and report method and report from there.
  // See task 2368334.
  if (!bpi->m_url.empty()) {
    client->error("@{url} cannot be specified alone.");
  } else {
    client->error("Breakpoint was not set in right format.");
  }
  return false;
}

// Carries out the Break command. This always involves an action on the
// client and usually, but not always, involves the server by sending
// this command to the server and waiting for its response.
bool CmdBreak::onClientImpl(DebuggerClient *client) {
  if (DebuggerCommand::onClientImpl(client)) return true;

  bool regex = false;
  BreakPointInfo::State state = BreakPointInfo::Always;

  int index = 1;
  if (client->arg(1, "regex")) {
    regex = true;
    index++;
  } else if (client->arg(1, "once")) {
    state = BreakPointInfo::Once;
    index++;
  } else if (client->arg(1, "list")) {
    return processList(client);
  } else if (hasStatusChangeArg(client)) {
    return processStatusChange(client);
  }

  string currentFile;
  int currentLine = 0;
  BreakPointInfoPtr loc = client->getCurrentLocation();
  if (loc) {
    currentFile = loc->m_file;
    currentLine = loc->m_line1;
  }

  BreakPointInfoPtr bpi;
  InterruptType interrupt = (InterruptType)(-1);

  if (client->argCount() == 0) {
    if (currentFile.empty() || currentLine == 0) {
      client->error("There is no current file or line to set breakpoint. "
                    "You will have to specify source file location yourself.");
      return true;
    }

    bpi = BreakPointInfoPtr(new BreakPointInfo(regex, state,
                                               currentFile.c_str(),
                                               currentLine));
  } else if (client->arg(index, "start")) {
    interrupt = RequestStarted;
  } else if (client->arg(index, "end")) {
    interrupt = RequestEnded;
  } else if (client->arg(index, "psp")) {
    interrupt = PSPEnded;
  } else {
    bpi = BreakPointInfoPtr(new BreakPointInfo(regex, state, BreakPointReached,
                                               client->argValue(index),
                                               currentFile));
  }

  if (interrupt >= 0) {
    string url;
    if (!client->arg(index + 1, "if") && !client->arg(index + 1, "&&")) {
      url = client->argValue(++index);
    }
    bpi = BreakPointInfoPtr(new BreakPointInfo(regex, state, interrupt, url));
  }

  if (!addToBreakpointListAndUpdateServer(client, bpi, index)) {
    client->tutorial(
      "This is the order of different arguments:\n"
      "\n"
      "\t[b]reak [o]nce {exp} if|&& {php}\n"
      "\n"
      "These are the components in a breakpoint {exp}:\n"
      "\n"
      "\tfile location: {file}:{line}\n"
      "\tfunction call: {func}()\n"
      "\tmethod invoke: {cls}::{method}()\n"
    );
  }
  return true;
}

static const StaticString s_id("id");
static const StaticString s_state("state");
static const StaticString s_is_exception("is_exception");
static const StaticString s_exception_class("exception_class");
static const StaticString s_file("file");
static const StaticString s_line1("line1");
static const StaticString s_line2("line2");
static const StaticString s_namespace("namespace");
static const StaticString s_func("func");
static const StaticString s_class("class");
static const StaticString s_url("url");
static const StaticString s_use_regex("use_regex");
static const StaticString s_clause_type("clause_type");
static const StaticString s_clause("clause");
static const StaticString s_if("if");
static const StaticString s_ampamp("ampamp");
static const StaticString s_desc("desc");

// Updates the client with information about the execution of this command.
// This information is not used by the command line client, but can be accessed
// via the debugger client API exposed to PHP programs.
void CmdBreak::setClientOutput(DebuggerClient *client) {
  // Output an array of current breakpoints including exceptions
  client->setOutputType(DebuggerClient::OTValues);
  Array values;
  m_breakpoints = client->getBreakPoints();
  for (int i = 0; i < (int)m_breakpoints->size(); i++) {
    BreakPointInfoPtr bpi = m_breakpoints->at(i);
    Array breakpoint;
    breakpoint.set(s_id, bpi->index());
    breakpoint.set(s_state, bpi->state(false));
    if (bpi->m_interrupt == ExceptionThrown) {
      breakpoint.set(s_is_exception, true);
      breakpoint.set(s_exception_class, bpi->getExceptionClass());
    } else {
      breakpoint.set(s_is_exception, false);
      breakpoint.set(s_file, bpi->m_file);
      breakpoint.set(s_line1, bpi->m_line1);
      breakpoint.set(s_line2, bpi->m_line2);
      breakpoint.set(s_namespace, bpi->getNamespace());
      breakpoint.set(s_func, bpi->getFunction());
      breakpoint.set(s_class, bpi->getClass());
    }
    breakpoint.set(s_url, bpi->m_url);
    breakpoint.set(s_use_regex, bpi->m_regex);
    breakpoint.set(s_clause_type, bpi->m_check ? s_if : s_ampamp);
    breakpoint.set(s_clause, bpi->m_clause);
    breakpoint.set(s_desc, bpi->desc());
    values.append(breakpoint);
  }
  client->setOTValues(values);
}

// Updates the breakpoint list in the proxy with the new list
// received from the client. Then sends the command back to the
// client as confirmation. Returns false if the confirmation message
// send failed.
bool CmdBreak::onServer(DebuggerProxy *proxy) {
  if (m_body == "update") {
    proxy->setBreakPoints(m_bps);
    m_breakpoints = &m_bps;
    return proxy->sendToClient(this);
  }
  return false;
}

// Returns true if the last command parsed by the client has
// an argument that changes the status of a breakpoint.
// I.e. clear, enable, disable or toggle.
bool CmdBreak::hasStatusChangeArg(DebuggerClient *client) {
  return
    hasClearArg(client) || hasEnableArg(client) ||
    hasDisableArg(client) || hasToggleArg(client);
}

// Returns true if the last command parsed by the client has
// the string "enable" in its first argument position.
bool CmdBreak::hasEnableArg(DebuggerClient *client) {
  return client->arg(1, "enable");
}

// Returns true if the last command parsed by the client has
// the string "disable" in its first argument position.
bool CmdBreak::hasDisableArg(DebuggerClient *client) {
  return client->arg(1, "disable");
}

// Returns true if the last command parsed by the client has
// the string "clear" in its first argument position.
bool CmdBreak::hasClearArg(DebuggerClient *client) {
  return client->arg(1, "clear");
}

// Returns true if the last command parsed by the client has
// the string "toggle" in its first argument position.
bool CmdBreak::hasToggleArg(DebuggerClient *client) {
  return client->arg(1, "toggle");
}

///////////////////////////////////////////////////////////////////////////////
}}
