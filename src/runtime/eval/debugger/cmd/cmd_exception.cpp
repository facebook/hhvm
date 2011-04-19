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

#include <runtime/eval/debugger/cmd/cmd_exception.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdException::list(DebuggerClient *client) {
  if (client->argCount() == 0) {
    client->addCompletion(DebuggerClient::AutoCompleteClasses);
    client->addCompletion("error");
    client->addCompletion("regex");
    client->addCompletion("once");
  } else if (client->arg(1, "regex") || client->arg(1, "once")) {
    client->addCompletion(DebuggerClient::AutoCompleteClasses);
  } else {
    client->addCompletion(DebuggerClient::AutoCompleteCode);
  }
}

bool CmdException::help(DebuggerClient *client) {
  client->helpTitle("Exception Command");
  client->helpCmds(
    "[e]xception {cls}",            "breaks if class of exception throws",
    "[e]xception {ns}::{cls}",      "breaks if class of exception throws",
    "[e]xception error",            "breaks on errors, warnings and notices",
    "[e]xception {above}@{url}",    "breaks only if url also matches",
    "",                             "",
    "[e]xception [r]egex {above}",  "breaks at matching regex pattern",
    "[e]xception [o]nce  {above}",  "breaks just once then disables it",
    "",                             "",
    "[e]xception {above} if {php}", "breaks if condition meets",
    "[e]xception {above} && {php}", "breaks and evaluates an expression",
    NULL
  );
  client->helpBody(
    "Exception command is similar to '[b]reak' command, except it's used "
    "to specify how to break on (or catch) a throw of an exception. Program "
    "stops right before the exception is about to throw. Resuming program "
    "execution will continue to throw the exception as is.\n"
    "\n"
    "Only a class name can be specified with an optional namespace. All "
    "exceptions of the class or its sub-classes will be matched. To specify "
    "a perfect match without sub-classing test, use '[e]xception [r]egex "
    "^{exact class name}$', although regex can match in a lot more different "
    "ways.\n"
    "\n"
    "An exception breakpoint can be listed, cleared or toggled with '[b]reak' "
    "commands."
  );
  return true;
}

bool CmdException::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;
  if (client->argCount() == 0) {
    return help(client);
  }

  bool regex = false;
  BreakPointInfo::State state = BreakPointInfo::Always;

  int index = 1;
  if (client->arg(1, "regex")) {
    regex = true;
    index++;
  } else if (client->arg(1, "once")) {
    state = BreakPointInfo::Once;
    index++;
  }

  BreakPointInfoPtr bpi(new BreakPointInfo(regex, state, ExceptionThrown,
                                           client->argValue(index), ""));
  if (!validate(client, bpi, index)) {
    client->tutorial(
      "This is the order of different arguments:\n"
      "\n"
      "\t[e]xception [r]egex|[o]nce {exp} if|&& {php}\n"
      "\n"
      "These are the components in an exception {exp}:\n"
      "\n"
      "\terror@{url}"
      "\t{namespace}::{cls}@{url}"
      "\n"
    );
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}}
