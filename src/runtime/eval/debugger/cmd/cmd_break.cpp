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

#include <runtime/eval/debugger/cmd/cmd_break.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdBreak::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
}

void CmdBreak::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
}

void CmdBreak::list(DebuggerClient *client) {
  if (client->argCount() == 0) {
    static const char *keywords1[] = {
      "clear",
      "end",
      "list",
      "once",
      "psp",
      "regex",
      "start",
      "toggle",
      NULL
    };
    client->addCompletion(keywords1);
    client->addCompletion(DebuggerClient::AUTO_COMPLETE_FILENAMES);
  }
}

bool CmdBreak::help(DebuggerClient *client) {
  client->helpTitle("Break Command");
  client->help("[b]reak                  breaks at current line of code ");
  client->help("[b]reak {file}:{line}    breaks at the line of the file ");
  client->help("[b]reak {line}           breaks at the line in current file");
  client->help("[b]reak {func}()         breaks at the function or class");
  client->help("[b]reak {cls}::{func}()  breaks at the method");
  client->help("[b]reak [r]egex ...      breaks if function or class matches");
  client->help("[b]reak [s]tart          breaks at start of web request");
  client->help("[b]reak [e]nd            breaks at end of web request");
  client->help("[b]reak [p]sp            breaks at end of psp");
  client->help("");
  client->help("[b]reak ... if ...       breaks if condition meets");
  client->help("[b]reak ... && ...       breaks and evaluates an expression");
  client->help("[b]reak [o]nce ...       breaks just once then disables it");
  client->help("");
  client->help("[b]reak [l]ist           lists all breakpoints");
  client->help("[b]reak [c]lear {num}    clears the n-th breakpoint on list");
  client->help("[b]reak [c]lear [a]ll    clears all breakpoints");
  client->help("[b]reak [c]lear          clears current breakpoint");
  client->help("[b]reak [t]oggle {num}   toggles the n-th breakpoint on list");
  client->help("[b]reak [t]oggle [a]ll   toggles all breakpoints");
  client->help("[b]reak [t]oggle         toggles current breakpoint");

  client->helpTitle("Where to break?");
  client->helpSection(
    "There are many ways to specify a source file location to set a "
    "breakpoint. For examples,\n"
    "\n"
    "\tb mypage.php:123\n"
    "\tb 456\n"
    "\tb foo()\n"
    "\tb MyClass::foo()\n"
    "\n"
    "\"file:function()\" and \"file:class::function()\" are also accepted, "
    "if there are more than one function or class method with the same name.\n"
    "\n"
    "\tb mypage.php:foo()\n"
    "\tb html/mypage.php:MyClass::foo()\n"
    "\n"
    "Normally this is not needed, as when there are more than one breakpoint "
    "that are found, it will prompt you to choose just one or all of them.\n"
    "\n"
    "You may also use regular expression to match these names. For examples,\n"
    "\n"
    "\tb r Feed.*::on.*()\n"
    "\n"
    "This may match FeedStory::onLoad(), FeedFilter::onclick(), etc.. Note "
    "that it uses PCRE format, not shell format. So you will have to use "
    "\".*\" instead of just \"*\" for wildcard match. Also, you will have to "
    "specify type of the name with one of those delimiters,\n"
    "\n"
    "\t{file}:\n"
    "\t{class}::\n"
    "\t{function}()"
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

  client->help("");
  return true;
}

bool CmdBreak::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;

  //TODO

  return help(client);
}

bool CmdBreak::onServer(DebuggerProxy *proxy) {
  ASSERT(false); // this command is processed entirely locally
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}}
