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
#include "hphp/runtime/ext/xdebug/xdebug_hook_handler.h"
#include "hphp/runtime/ext/xdebug/xdebug_utils.h"

#include "hphp/runtime/ext/url/ext_url.h"

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

  // run never responds immediately
  bool shouldRespond() const override { return false; }

  bool isValidInStatus(Status status) const override {
    return
      status == Status::STARTING ||
      status == Status::STOPPING ||
      status == Status::BREAK;
  }

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
};

////////////////////////////////////////////////////////////////////////////////
// step_into -i #
// steps to the next statement, if there is a function call involved it will
// break on the first statement in that function

class StepIntoCmd : public XDebugCommand {
public:
  StepIntoCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~StepIntoCmd() {}

  // Respond on step break
  bool shouldRespond() const override { return false; }

  bool isValidInStatus(Status status) const override {
    return
      status == Status::STARTING ||
      status == Status::STOPPING ||
      status == Status::BREAK;
  }

  bool handleImpl(xdebug_xml_node& xml) const override {
    phpDebuggerStepIn();
    return true;
  }
};

////////////////////////////////////////////////////////////////////////////////
// breakpoint_set -i # [-t TYPE] [-s STATE] [-f FILENAME] [-n LINENO]
//                     [-m FUNCTION] [-x EXCEPTION] [-h HIT_VALUE]
//                     [-o HIT_CONDITION] [-r 0|1] [-- EXPRESSION]
// Adds a breakpoint

// Valid breakpoint strings
static const StaticString
  s_LINE("line"),
  s_CONDITIONAL("conditional"),
  s_CALL("call"),
  s_RETURN("return"),
  s_EXCEPTION("exception"),
  s_WATCH("watch"),
  s_ENABLED("enabled"),
  s_DISABLED("disabled"),
  s_GREATER_OR_EQUAL(">="),
  s_EQUAL("=="),
  s_MOD("%");

class BreakpointSetCmd : public XDebugCommand {
public:
  BreakpointSetCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {
    XDebugBreakpoint& bp = m_breakpoint;

    // Type is required
    if (args['t'].isNull()) {
      throw XDebugServer::ERROR_INVALID_ARGS;
    }

    // Type: line|call|return|exception|conditional
    String typeName = args['t'].toString();
    if (typeName == s_LINE || typeName == s_CONDITIONAL) {
      // Despite spec, line and conditional are the same in php5 xdebug
      bp.type = XDebugBreakpoint::Type::LINE;
    } else if (typeName == s_CALL) {
      bp.type = XDebugBreakpoint::Type::CALL;
    } else if (typeName == s_RETURN) {
      bp.type = XDebugBreakpoint::Type::RETURN;
    } else if (typeName == s_EXCEPTION) {
      bp.type = XDebugBreakpoint::Type::EXCEPTION;
    } else if (typeName == s_WATCH) {
      throw XDebugServer::ERROR_BREAKPOINT_TYPE_NOT_SUPPORTED;
    } else {
      throw XDebugServer::ERROR_INVALID_ARGS;
    }

    // State: enabled|disabled
    if (!args['s'].isNull()) {
      String state = args['s'].toString();
      if (state == s_ENABLED) {
        bp.enabled = true;
      } else if (state == s_DISABLED) {
        bp.enabled = false;
      } else {
        throw XDebugServer::ERROR_INVALID_ARGS;
      }
    }

    // Hit condition and value. php5 xdebug does not throw an error if only
    // one of the two are provided
    if (!args['h'].isNull() && !args['o'].isNull()) {
      String condition = args['o'].toString();
      String val = args['h'].toString();
      if (condition == s_GREATER_OR_EQUAL) {
        bp.hitCondition = XDebugBreakpoint::HitCondition::GREATER_OR_EQUAL;
      } else if (condition == s_EQUAL) {
        bp.hitCondition = XDebugBreakpoint::HitCondition::EQUAL;
      } else if (condition == s_MOD) {
        bp.hitCondition = XDebugBreakpoint::HitCondition::MULTIPLE;
      } else {
        throw XDebugServer::ERROR_INVALID_ARGS;
      }
      bp.hitValue = strtol(val.data(), nullptr, 10);
    }

    // Temporary: 0|1 -- xdebug actually just throws the passed in data into
    // strtol.
    if (!args['r'].isNull()) {
      String temp = args['r'].toString();
      m_breakpoint.temporary = (bool) strtol(temp.data(), nullptr, 10);
    }

    // Initialize line breakpoint
    if (bp.type == XDebugBreakpoint::Type::LINE) {
      // Grab the line #
      if (args['n'].isNull()) {
        throw XDebugServer::ERROR_INVALID_ARGS;
      }
      bp.line = strtol(args['n'].toString().data(), nullptr, 10);

      // Grab the file, use the current if none provided
      if (args['f'].isNull()) {
        StringData* filename = g_context->getContainingFileName();
        if (filename == staticEmptyString()) {
          throw XDebugServer::ERROR_STACK_DEPTH_INVALID;
        }
        bp.fileName = String(filename);
      } else {
        bp.fileName = XDebugUtils::pathFromUrl(args['f'].toString());
      }

      // Ensure consistency between filenames
      bp.fileName = File::TranslatePath(bp.fileName);

      // Grab the condition string if one was provided
      if (!args['-'].isNull()) {
        Variant cond_var = HHVM_FN(base64_decode)(args['-'].toString(), false);
        if (cond_var.isString()) {
          bp.condition = cond_var.toString();
        }
      }
    }

    // Call and return type
    if (bp.type == XDebugBreakpoint::Type::CALL ||
        bp.type == XDebugBreakpoint::Type::RETURN) {
      if (args['m'].isNull()) {
        throw XDebugServer::ERROR_INVALID_ARGS;
      }
      bp.funcName = args['m'].toString();

      // This is in php5 xdebug, but not in the spec. If 'a' is passed, the
      // value is expected to be a class name that will be prepended to the
      // passed method name.
      if (!args['a'].isNull()) {
        bp.className = args['a'];
      }

      // Precompute full function name
      bp.fullFuncName = bp.className.isNull() ?
        bp.funcName : (bp.className.toString() + "::" + bp.funcName);
    }

    // Exception type
    if (bp.type == XDebugBreakpoint::Type::EXCEPTION) {
      if (args['x'].isNull()) {
        throw XDebugServer::ERROR_INVALID_ARGS;
      }
      bp.exceptionName = args['x'].toString();
      return;
    }
  }

  ~BreakpointSetCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    // Add the breakpoint, write out the id
    int id = XDEBUG_ADD_BREAKPOINT(m_breakpoint);
    xdebug_xml_add_attribute_ex(&xml, "id", xdebug_sprintf("%d", id), 0, 1);

    // Add the breakpoint state
    if (m_breakpoint.enabled) {
      xdebug_xml_add_attribute(&xml, "state", "enabled");
    } else {
      xdebug_xml_add_attribute(&xml, "state", "disabled");
    }
    return false;
  }

private:
  // Breakpoint manipulated by addopt
  XDebugBreakpoint m_breakpoint;
};

////////////////////////////////////////////////////////////////////////////////
// XDebugCommand implementation

XDebugCommand::XDebugCommand(XDebugServer& server,
                             const String& cmd,
                             const Array& args)
  : m_server(server), m_commandStr(cmd) {
  // A transaction id must be provided
  if (args['i'].isNull()) {
    throw XDebugServer::ERROR_INVALID_ARGS;
  }
  m_transactionId = args['i'].toString();
}

bool XDebugCommand::handle(xdebug_xml_node& response) const {
  m_server.addCommand(response, *this);
  return handleImpl(response);
}

const XDebugCommand* XDebugCommand::fromString(XDebugServer& server,
                                               const String& cmdStr,
                                               const Array& args) {
  XDebugCommand* cmd;
  if (cmdStr == s_CMD_STATUS) {
    cmd = new StatusCmd(server, cmdStr, args);
  } else if (cmdStr == s_CMD_RUN) {
    cmd = new RunCmd(server, cmdStr, args);
  } else if (cmdStr == s_CMD_BREAKPOINT_SET) {
    cmd = new BreakpointSetCmd(server, cmdStr, args);
  } else if (cmdStr == s_CMD_STEP_INTO) {
    cmd = new StepIntoCmd(server, cmdStr, args);
  } else {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
  }

  // Ensure this command is valid in the given server status
  Status status; Reason reason;
  server.getStatus(status, reason);
  if (!cmd->isValidInStatus(status)) {
    delete cmd;
    throw XDebugServer::ERROR_COMMAND_UNAVAILABLE;
  }
  return cmd;
}

///////////////////////////////////////////////////////////////////////////////
}
