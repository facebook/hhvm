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

#include "hphp/runtime/ext/xdebug/xdebug_command.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// Helpers

// Command strings
static const StaticString
  s_CMD_STATUS("status"),
  s_CMD_FEATURE_GET("feature_get"),
  s_CMD_FEATURE_SET("feature_set"),
  s_CMD_RUN("run"),
  s_CMD_STEP_INTO("step_into"),
  s_CMD_STEP_OVER("step_over"),
  s_CMD_STOP("stop"),
  s_CMD_DETACH("detach"),
  s_CMD_BREAKPOINT_SET("breakpoint_set"),
  s_CMD_BREAKPOINT_GET("breakpoint_get"),
  s_CMD_BREAKPOINT_UPDATE("breakpoint_update"),
  s_CMD_BREAKPOINT_REMOVE("breakpoint_remove"),
  s_CMD_STACK_DEPTH("stack_depth"),
  s_CMD_STACK_GET("stack_get"),
  s_CMD_CONTEXT_NAMES("context_names"),
  s_CMD_CONTEXT_GET("context_get"),
  s_CMD_PROPERTY_GET("property_get"),
  s_CMD_PROPERTY_SET("property_set"),
  s_CMD_PROPERTY_VALUE("property_value"),
  s_CMD_SOURCE("source"),
  s_CMD_STDOUT("stdout"),
  s_CMD_STDERR("stderr"),
  s_CMD_EVAL("eval"),
  s_CMD_PROFILER_NAME_GET("xcmd_profiler_name_get"),
  s_CMD_GET_EXECUTABLE_LINES("xcmd_get_executable_lines");

// These are used a lot, prevent unnecessary verbosity
typedef XDebugServer::Status Status;
typedef XDebugServer::Reason Reason;

////////////////////////////////////////////////////////////////////////////////
// status -i #
// Returns the status of the server

class StatusCmd : public XDebugCommand {
public:
  StatusCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~StatusCmd() {}

  bool isValidInStatus(Status status) const override {
    return true;
  }

  bool handleImpl(xdebug_xml_node& xml) const override {
    m_server.addStatus(xml);
    return false;
  }
};

////////////////////////////////////////////////////////////////////////////////
// run -i #
// Runs the program until a breakpoint is hit or the script is finished

class RunCmd : public XDebugCommand {
public:
  RunCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~RunCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    // Get the server status
    Status status;
    Reason reason;
    m_server.getStatus(status, reason);

    // Modify the status
    switch (status) {
      case Status::STARTING:
      case Status::BREAK:
        m_server.setStatus(Status::RUNNING, Reason::OK);
        break;
      case Status::STOPPING:
        m_server.setStatus(Status::DETACHED, Reason::OK);
        break;
      default:
        throw Exception("Command 'run' invalid in this server state.");
    }
    return true;
  }

  // run never responds immediatly
  bool shouldRespond() const override {
    return false;
  }

  bool isValidInStatus(Status status) const override {
    return
      status == Status::STARTING ||
      status == Status::STOPPING ||
      status == Status::BREAK;
  }
};

////////////////////////////////////////////////////////////////////////////////
// XDebugCommand implementation

XDebugCommand::XDebugCommand(XDebugServer& server,
                             const String& cmd,
                             const Array& args)
  : m_server(server), m_commandStr(cmd) {
  // A transaction id must be provided
  if (args['i'].isNull()) {
    throw InvalidArgs::InvalidArgs;
  }

  // Add each option
  for (ArrayIter iter(args); iter; ++iter) {
    char opt = iter.first().toByte();
    const String val = iter.second().toString();
    if (!addOpt(opt, val)) {
      throw InvalidArgs::InvalidArgs;
    }
  }
}

bool XDebugCommand::addOpt(char opt, const String& val) {
  switch (opt) {
    case 'i':
      m_transactionId = val;
      return true;
    default:
      return addOptImpl(opt, val);
  }
}

bool XDebugCommand::handle(xdebug_xml_node& response) const {
  m_server.addCommand(response, *this);
  return handleImpl(response);
}

const XDebugCommand* XDebugCommand::fromString(XDebugServer& server,
                                               const String& cmdStr,
                                               const Array& args) {
  const XDebugCommand* cmd;
  if (cmdStr == s_CMD_STATUS) {
    cmd = new StatusCmd(server, cmdStr, args);
  } else if (cmdStr == s_CMD_RUN) {
    cmd = new RunCmd(server, cmdStr, args);
  } else {
    throw InvalidCommandString::InvalidCommandString;
  }

  // Ensure this command is valid in the given server status. We can't do this
  // in the constructor because virtuals are not yet initialized
  Status status; Reason reason;
  server.getStatus(status, reason);
  if (!cmd->isValidInStatus(status)) {
    delete cmd;
    throw InvalidStatus::InvalidStatus;
  }

  return cmd;
}

///////////////////////////////////////////////////////////////////////////////
}
