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

using namespace std;

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
      NULL
    };
    client->addCompletion(keywords1);
    client->addCompletion(DebuggerClient::AutoCompleteFileNames);
    client->addCompletion(DebuggerClient::AutoCompleteFunctions);
    client->addCompletion(DebuggerClient::AutoCompleteClasses);
    client->addCompletion(DebuggerClient::AutoCompleteClassMethods);
  } else if (client->argCount() == 1) {
    if (client->arg(1, "clear") || client->arg(1, "toggle")) {
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
    "[b]reak [s]tart {url}",    "breaks at start of web request",
    "[b]reak [e]nd   {url}",    "breaks at end of web request",
    "[b]reak [p]sp   {url}",    "breaks at end of psp",
    "",                         "",
    "[b]reak [r]egex {above}",  "breaks at matching regex pattern",
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
    NULL
  );

  client->helpTitle("Where to break?");
  client->helpSection(
    "There are many ways to specify a source file location to set a "
    "breakpoint, but it's ONE single string without whitespaces. The complete "
    "format, though every field is optional, looks like this,\n"
    "\n"
    "\t{file location}:{call}=>{call}()@{url}\n"
    "\t{call}=>{call}():{file location}@{url}\n"
    "\n"
    "\tfile location: {file}:{line1}-{line2}\n"
    "\tfunction call: {namespace}::{cls}::{func}\n"
    "\turl matching:  @{url}\n"
    "\n"
    "1) Url has to be specified at end.\n"
    "\n"
    "2) Function calls can be 1, 2 or more, matching a call chain. If more "
    "than one function are specified, they don't have to be direct callers "
    "to match. It will match any caller on the stack.\n"
    "\n"
    "3) Pay attention to those delimiters, and they are required to tell "
    "what a field should be interpreted as, unless it is a number, then it "
    "must be line numbers. Otherwise, use them to indicate what the names "
    "are:\n"
    "\n"
    "\t{file}:                filename\n"
    "\t{line1}-{line2}        any line between them (inclusive)\n"
    "\t{line}                 single line, if without dashes around\n"
    "\t{line}:                needs colon if anything after\n"
    "\t{namespace}::{cls}::   a class in specified namespace\n"
    "\t{cls}::                a class in any namespace\n"
    "\t{func}()               function or method\n"
    "\t{func}=>{func}()       function called by specified function\n"
    "\t{cls}::{method}()      class method (static or instance)\n"
    "\t@{url}                 breaks only when this URL is visited\n"
    "\n"
    "For examples,\n"
    "\n"
    "\tb mypage.php:123\n"
    "\tb 456\n"
    "\tb foo()\n"
    "\tb MyClass::foo()\n"
    "\tb mypage.php:foo()\n"
    "\tb html/mypage.php:MyClass::foo()\n"
    "\tb mypage.php:123@index.php\n"
    "\n"
    "4) You may also use regular expressions to match any of these names, "
    "except line numbers. For examples,\n"
    "\n"
    "\tb r Feed.*::on.*()\n"
    "\n"
    "This may match FeedStory::onLoad(), FeedFilter::onclick(), etc.. Note "
    "that it uses PCRE format, not shell format. So you will have to use "
    "\".*\" instead of just \"*\" for wildcard match."
  );

  client->helpTitle("Special Breakpoints");
  client->helpSection(
    "There are special breakpoints that can only be set by names:\n"
    "\n"
    "\tstart\n"
    "\tend\n"
    "\tpsp\n"
    "\n"
    "They represent different time points of a web request. \"start\" is at "
    "beginning of a web request, when no PHP file is invoked yet, but query "
    "strings and server variables are already prepared. \"end\" is at end of "
    "a web request, but BEFORE post-send processing (psp). \"psp\" is at "
    "END of psp, not beginning. To set a breakpoint at beginning of psp, use "
    "\"end\", because end of a request is the same as beginning of psp."
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
        if (client->arg(1, "clear")) {
          action = "cleared";
          bps->erase(bps->begin() + index);
        } else {
          ASSERT(client->arg(1, "toggle"));
          action = "updated";
          bp->toggle();
        }
        client->info("Breakpoint %d is %s %s", bp->index(),
                     action, bp->site().c_str());
        found = true;
      }
    }
    if (found) {
      m_body = "update";
      client->send(this);
      return true;
    }

    client->error("There is no current breakpoint to clear or toggle.");
    return true;
  }

  if (client->arg(2, "all")) {
    if (client->arg(1, "clear")) {
      m_breakpoints->clear();
      m_body = "update";
      client->send(this);
      client->info("All breakpoints are cleared.");
      return true;
    }

    ASSERT(client->arg(1, "toggle"));
    for (unsigned int i = 0; i < m_breakpoints->size(); i++) {
      BreakPointInfoPtr bpi = (*m_breakpoints)[i];
      bpi->toggle();
    }
    m_body = "update";
    client->send(this);
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
  if (client->arg(1, "clear")) {
    m_breakpoints->erase(m_breakpoints->begin() + index);
    m_body = "update";
    client->send(this);
    client->info("Breakpoint %d cleared %s", bpi->index(),
                 bpi->desc().c_str());
  } else {
    ASSERT(client->arg(1, "toggle"));
    bpi->toggle();
    m_body = "update";
    client->send(this);
    client->info("Breakpoint %d's state is changed to %s.", bpi->index(),
                 bpi->state(false).c_str());
  }

  return true;
}

bool CmdBreak::update(DebuggerClient *client) {
  m_body = "update";
  m_breakpoints = client->getBreakPoints();
  client->send(this);
  return true;
}

bool CmdBreak::validate(DebuggerClient *client, BreakPointInfoPtr bpi,
                        int index) {
  ++index;
  if (client->arg(index, "if")) {
    bpi->setClause(client->argRest(++index), true);
  } else if (client->arg(index, "&&")) {
    bpi->setClause(client->argRest(++index), false);
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
    m_body = "update";
    client->send(this);
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
  } else if (client->arg(1, "clear") || client->arg(1, "toggle")) {
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
      "\t[b]reak [r]egex|[o]nce {special}|{exp} if|&& {php}\n"
      "\n"
      "These are the components in a breakpoint {exp}:\n"
      "\n"
      "\t{file location}:{call}=>{call}()@{url}\n"
      "\t{call}=>{call}():{file location}@{url}\n"
      "\n"
      "\tfile location: {file}:{line1}-{line2}\n"
      "\tfunction call: {namespace}::{cls}::{func}\n"
      "\turl matching:  @{url}\n"
    );
  }
  return true;
}

bool CmdBreak::onServer(DebuggerProxy *proxy) {
  if (m_body == "update") {
    proxy->setBreakPoints(m_bps);
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}}
