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

#include <runtime/eval/debugger/cmd/cmd_break.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdBreak::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  ASSERT(m_breakpoints);
  BreakPointInfo::SendImpl(*m_breakpoints, thrift);
}

void CmdBreak::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  BreakPointInfo::RecvImpl(m_bps, thrift);
}

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
      NULL
    };
    client->addCompletion(keywords1);
    client->addCompletion(DebuggerClient::AutoCompleteFileNames);
    client->addCompletion(DebuggerClient::AutoCompleteFunctions);
    client->addCompletion(DebuggerClient::AutoCompleteClasses);
    client->addCompletion(DebuggerClient::AutoCompleteClassMethods);
  } else if (client->argCount() == 1) {
    if (hasUpdateArg(client)) {
      client->addCompletion("all");
    } else if (!client->arg(1, "list")) {
      client->addCompletion(DebuggerClient::AutoCompleteFileNames);
    }
  } else {
    client->addCompletion(DebuggerClient::AutoCompleteCode);
  }
}

bool CmdBreak::help(DebuggerClient *client) {
  client->helpTitle("Break Command");
  client->helpCmds(
    "[b]reak",                  "breaks at current line of code",
    "[b]reak {exp}",            "breaks at matching location",
    "",                         "",
    "[b]reak [o]nce  {above}",  "breaks just once then disables it",
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
    NULL
  );

  client->helpTitle("Where to break?");
  client->helpSection(
    "There are many ways to specify a source file location to set a "
    "breakpoint, but it's ONE single string without whitespaces. The"
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

  client->helpTitle("Conditional Breakpoints and Watchpoints");
  client->helpSection(
    "Every breakpoint can specify a condition, which is an arbitrary PHP "
    "expression that will be evaulated to TRUE or FALSE. When TRUE, it will "
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

bool CmdBreak::processUpdate(DebuggerClient *client) {
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
          ASSERT(hasToggleArg(client));
          action = "updated";
          bp->toggle();
        }
        client->info("Breakpoint %d is %s %s", bp->index(),
                     action, bp->site().c_str());
        found = true;
      }
    }
    if (found) {
      updateImpl(client);
      return true;
    }

    client->error("There is no current breakpoint to clear or toggle.");
    return true;
  }

  if (client->arg(2, "all")) {
    if (hasClearArg(client)) {
      m_breakpoints->clear();
      updateImpl(client);
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
        ASSERT(hasToggleArg(client));
        bpi->toggle();
      }
    }

    updateImpl(client);
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
    updateImpl(client);
    client->info("Breakpoint %d cleared %s", bpi->index(),
                 bpi->desc().c_str());
  } else if (hasEnableArg(client)) {
    bpi->setState(BreakPointInfo::Always);
    updateImpl(client);
    client->info("Breakpoint %d's state is changed to %s.", bpi->index(),
                 bpi->state(false).c_str());
  } else if (hasDisableArg(client)) {
    bpi->setState(BreakPointInfo::Disabled);
    updateImpl(client);
    client->info("Breakpoint %d's state is changed to %s.", bpi->index(),
                 bpi->state(false).c_str());
  } else {
    ASSERT(hasToggleArg(client));
    bpi->toggle();
    updateImpl(client);
    client->info("Breakpoint %d's state is changed to %s.", bpi->index(),
                 bpi->state(false).c_str());
  }

  return true;
}

bool CmdBreak::updateImpl(DebuggerClient *client) {
  m_body = "update";
  client->xend<CmdBreak>(this);
  return true;
}

bool CmdBreak::update(DebuggerClient *client) {
  m_breakpoints = client->getBreakPoints();
  return updateImpl(client);
}

bool CmdBreak::validate(DebuggerClient *client, BreakPointInfoPtr bpi,
                        int index) {
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
    updateImpl(client);
    client->info("Breakpoint %d set %s", bpi->index(), bpi->desc().c_str());
    return true;
  }

  if (!bpi->m_url.empty()) {
    client->error("@{url} cannot be specified alone.");
  } else {
    client->error("Breakpoint was not set in right format.");
  }
  return false;
}

bool CmdBreak::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;

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
  } else if (hasUpdateArg(client)) {
    return processUpdate(client);
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

  if (!validate(client, bpi, index)) {
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

void CmdBreak::setClientOutput(DebuggerClient *client) {
  // Output an array of current breakpoints including exceptions
  client->setOutputType(DebuggerClient::OTValues);
  Array values;
  m_breakpoints = client->getBreakPoints();
  for (int i = 0; i < (int)m_breakpoints->size(); i++) {
    BreakPointInfoPtr bpi = m_breakpoints->at(i);
    Array breakpoint;
    breakpoint.set("id", bpi->index());
    breakpoint.set("state", bpi->state(false));
    if (bpi->m_interrupt == ExceptionThrown) {
      breakpoint.set("is_exception", true);
      breakpoint.set("exception_class", bpi->getExceptionClass());
    } else {
      breakpoint.set("is_exception", false);
      breakpoint.set("file", bpi->m_file);
      breakpoint.set("line1", bpi->m_line1);
      breakpoint.set("line2", bpi->m_line2);
      breakpoint.set("namespace", bpi->getNamespace());
      breakpoint.set("func", bpi->getFunction());
      breakpoint.set("class", bpi->getClass());
    }
    breakpoint.set("url", bpi->m_url);
    breakpoint.set("use_regex", bpi->m_regex);
    breakpoint.set("clause_type", bpi->m_check ? "if" : "&&");
    breakpoint.set("clause", bpi->m_clause);
    breakpoint.set("desc", bpi->desc());
    values.append(breakpoint);
  }
  client->setOTValues(values);
}

bool CmdBreak::onServer(DebuggerProxy *proxy) {
  if (m_body == "update") {
    if (!m_bps.empty()) {
      RequestInjectionData &rjdata =
        ThreadInfo::s_threadInfo->m_reqInjectionData;
      rjdata.debuggerIdle = 0;
    }
    proxy->setBreakPoints(m_bps);
    m_breakpoints = &m_bps;
    return proxy->send(this);
  }
  return false;
}

bool CmdBreak::hasUpdateArg(DebuggerClient *client) {
  return
    hasClearArg(client) || hasEnableArg(client) ||
    hasDisableArg(client) || hasToggleArg(client);
}

bool CmdBreak::hasEnableArg(DebuggerClient *client) {
  return client->arg(1, "enable");
}

bool CmdBreak::hasDisableArg(DebuggerClient *client) {
  return client->arg(1, "disable");
}

bool CmdBreak::hasClearArg(DebuggerClient *client) {
  return client->arg(1, "clear");
}

bool CmdBreak::hasToggleArg(DebuggerClient *client) {
  return client->arg(1, "toggle");
}

///////////////////////////////////////////////////////////////////////////////
}}
