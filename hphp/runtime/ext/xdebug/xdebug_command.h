/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_XDEBUG_COMMAND_H_
#define incl_HPHP_XDEBUG_COMMAND_H_

#include "hphp/runtime/ext/xdebug/status.h"

#include <string>

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

struct Array;
struct String;
struct XDebugServer;

struct xdebug_xml_node;

/*
 * Base class of all commands.  An instance of an xdebug command is alive until
 * the next command is received.
 */
struct XDebugCommand {
  /*
   * Given an xdebug server, a command string, and command arguments, constructs
   * and returns a new XDebugCommand corresponding to the given string.  This is
   * how commands should be created.
   */
  static XDebugCommand* fromString(XDebugServer& server,
                                   const String& cmdStr,
                                   const Array& args);
  /*
   * Internal constructor used by fromString.  It should never be called
   * explicitly.  This is where arguments should be parsed.  Note that php5
   * xdebug doesn't actually raise an error on extra/invalid args.
   */
  XDebugCommand(XDebugServer& server, const String& cmdStr, const Array& args);
  virtual ~XDebugCommand() {}

  /*
   * Perform the command, outputting data to the passed xml node.  This case
   * class automatically adds standard info to the response, the subclass should
   * override and implement handleImpl.  If the xdebug should continue script
   * exection after after this command completes (as in break out of the command
   * loop), this should return true.  Otherwise, this return false.
   */
  bool handle(xdebug_xml_node& response);
  virtual void handleImpl(xdebug_xml_node& response) = 0;

  /*
   * Returns true if this command should return a response to the client.  For
   * almost all commands, this is true, so it defaults to true.
   */
  virtual bool shouldRespond() const { return true; }

  /*
   * Returns true if this command should cause the server to continue execution.
   * This will always be called after handleImpl.
   */
  virtual bool shouldContinue() const { return false; }

  const std::string& getCommandStr() const { return m_commandStr; }
  const std::string& getTransactionId() const { return m_transactionId; }

  /*
   * Returns true if this command is valid in the given server status.  Almost
   * all commands are valid except for when the server is stopping, so this is
   * the default.
   */
  virtual bool isValidInStatus(XDebugStatus status) const {
    return status != XDebugStatus::Stopping;
  }

protected:
  /* The server needs to be accesible by all children. */
  XDebugServer& m_server;

private:
  /*
   * These are std::string instead of String because XDebugCommand objects are
   * shared across two threads: the request thread, and the polling thread.
   */
  std::string m_commandStr;
  std::string m_transactionId;
};

////////////////////////////////////////////////////////////////////////////////
}

#endif
