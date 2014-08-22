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
#include "hphp/system/constants.h"

namespace HPHP {

////////////////////////////////////////////////////////////////////////////////
// Commands

// COMMAND(NAME, CLASS)
//  NAME is the command name
//  CLASS is the corresponding class
#define COMMANDS \
  COMMAND("status", StatusCmd)                                                 \
  COMMAND("feature_get", FeatureGetCmd)                                        \
  COMMAND("feature_set", FeatureSetCmd)                                        \
  COMMAND("run", RunCmd)                                                       \
  COMMAND("step_into", StepIntoCmd)                                            \
  COMMAND("step_out", StepOutCmd)                                              \
  COMMAND("step_over", StepOverCmd)                                            \
  COMMAND("stop", StopCmd)                                                     \
  COMMAND("detach", DetachCmd)                                                 \
  COMMAND("breakpoint_set", BreakpointSetCmd)                                  \
  COMMAND("breakpoint_get", BreakpointGetCmd)                                  \
  COMMAND("breakpoint_update", BreakpointUpdateCmd)                            \
  COMMAND("breakpoint_remove", BreakpointRemoveCmd)                            \
  COMMAND("stack_depth", StackDepthCmd)                                        \
  COMMAND("stack_get", StackGetCmd)                                            \
  COMMAND("context_names", ContextNamesCmd)                                    \
  COMMAND("context_get", ContextGetCmd)                                        \
  COMMAND("property_get", PropertyGetCmd)                                      \
  COMMAND("property_set", PropertySetCmd)                                      \
  COMMAND("property_value", PropertyValueCmd)                                  \
  COMMAND("source", SourceCmd)                                                 \
  COMMAND("stdout", StdoutCmd)                                                 \
  COMMAND("stderr", StderrCmd)                                                 \
  COMMAND("eval", EvalCmd)                                                     \
  COMMAND("xcmd_profiler_name_get", ProfilerNameGetCmd)                        \

////////////////////////////////////////////////////////////////////////////////
// Features
// See http://xdebug.org/docs-dbgp.php#feature-names for a complete list

// FEATURE(NAME, SUPPORTED, VALUE, FREE)
//  NAME is the feature name
//  SUPPORTED is whether or not the feature is supported
//  VALUE is the value to add to the xml node
//  FREE is whether or not the value should be freed
#define FEATURES                                                               \
  FEATURE("breakpoint_languages", "0", nullptr, false)                         \
  FEATURE("breakpoint_types", "1",                                             \
          "line conditional call return exception", false)                     \
  FEATURE("data_encoding", "0", nullptr, false)                                \
  FEATURE("encoding", "1", "iso-8859-1",  false)                               \
  FEATURE("language_name", "1", "PHP", false)                                  \
  FEATURE("language_supports_threads", "1", "0", false)                        \
  FEATURE("language_version", "1", xdstrdup(k_HHVM_VERSION.c_str()), true)     \
  FEATURE("max_children", "1",                                                 \
          xdebug_sprintf("%d", m_server.m_maxChildren), true)                  \
  FEATURE("max_data", "1", xdebug_sprintf("%d", m_server.m_maxData), true)     \
  FEATURE("max_depth", "1", xdebug_sprintf("%d", m_server.m_maxDepth), true)   \
  FEATURE("protocol_version", "1", DBGP_VERSION, false)                        \
  FEATURE("supported_encodings", "1", "iso-8859-1", false)                     \
  FEATURE("supports_async", "1", "0", false)                                   \
  FEATURE("supports_postmortem", "1", "1", false)                              \
  FEATURE("show_hidden", "1",                                                  \
          xdebug_sprintf("%d", m_server.m_showHidden), true)                   \

////////////////////////////////////////////////////////////////////////////////

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
// feature_get -i # -n NAME

class FeatureGetCmd : public XDebugCommand {
public:
  FeatureGetCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {
    // Feature name is required
    if (args['n'].isNull()) {
      throw XDebugServer::ERROR_INVALID_ARGS;
    }
    m_feature = args['n'].toString().toCppString();
  }

  ~FeatureGetCmd() {}

  bool isValidInStatus(Status status) const override {
    return true;
  }

  bool handleImpl(xdebug_xml_node& xml) const override {
    // Set to true once we have a match. Const cast is needed due to xdebug
    // xml api.
    bool match = false;
    xdebug_xml_add_attribute(&xml, "feature_name",
                             const_cast<char*>(m_feature.c_str()));

    // Check against the defined features
    #define FEATURE(name, supported, val, free)                                \
      if (!match && m_feature == name) {                                       \
        if (val != nullptr) {                                                  \
          xdebug_xml_add_text(&xml, val, free);                                \
        }                                                                      \
        xdebug_xml_add_attribute(&xml, "supported", supported);                \
        match = true;                                                          \
      }
    FEATURES
    #undef FEATURE

    // Check against the commands
    #define COMMAND(name, className)                                           \
      if (!match && m_feature == name) {                                       \
        xdebug_xml_add_text(&xml, "1", 0);                                     \
        xdebug_xml_add_attribute(&xml, "supported", "1");                      \
        match = true;                                                          \
      }
    COMMANDS
    #undef COMMAND

    // Unknown feature name
    if (!match) {
      xdebug_xml_add_text(&xml, "0", 0);
      xdebug_xml_add_attribute(&xml, "supported", "0");
    }
    return false;
  }

private:
  string m_feature;
};

////////////////////////////////////////////////////////////////////////////////
// feature_set -i # -n NAME -v VALUE

class FeatureSetCmd : public XDebugCommand {
public:
  FeatureSetCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {
    // Feature name is required
    if (args['n'].isNull()) {
      throw XDebugServer::ERROR_INVALID_ARGS;
    }
    m_feature = args['n'].toString().toCppString();

    // Value is required
    if (args['v'].isNull()) {
      throw XDebugServer::ERROR_INVALID_ARGS;
    }
    m_value = args['v'].toString().toCppString();
  }

  ~FeatureSetCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    const char* value_str = m_value.c_str();

    // These could be thrown into a macro, but there aren't very many cases
    if (m_feature == "max_children") {
      m_server.m_maxChildren = strtol(value_str, nullptr, 10);
    } else if (m_feature == "max_data") {
      m_server.m_maxData = strtol(value_str, nullptr, 10);
    } else if (m_feature == "max_depth") {
      m_server.m_maxDepth = strtol(value_str, nullptr, 10);
    } else if (m_feature == "show_hidden") {
      m_server.m_showHidden = strtol(value_str, nullptr, 10);
    } else if (m_feature == "multiple_sessions") {
      // php5 xdebug doesn't do anything here with this value, but it is doesn't
      // throw an error, either
    } else if (m_feature == "encoding") {
      if (m_value != "iso-8859-1") {
        throw XDebugServer::ERROR_ENCODING_NOT_SUPPORTED;
      }
    } else {
      throw XDebugServer::ERROR_INVALID_ARGS;
    }

    // Const cast is needed due to xdebug xml api.
    xdebug_xml_add_attribute(&xml, "feature",
                             const_cast<char*>(m_feature.c_str()));
    xdebug_xml_add_attribute(&xml, "success", "1");
    return false;
  }

private:
  string m_feature;
  string m_value;
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

    // Call the debugger hook and continue
    phpDebuggerContinue();
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
// step_out -i #
// steps out of the current scope and breaks on the statement after returning
// from the current function.

class StepOutCmd : public XDebugCommand {
public:
  StepOutCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~StepOutCmd() {}

  // Respond on step out break
  bool shouldRespond() const override { return false; }

  bool isValidInStatus(Status status) const override {
    return
      status == Status::STARTING ||
      status == Status::STOPPING ||
      status == Status::BREAK;
  }

  bool handleImpl(xdebug_xml_node& xml) const override {
    phpDebuggerStepOut();
    return true;
  }
};

////////////////////////////////////////////////////////////////////////////////
// step_over -i #
// steps to the next line. Steps over function calls.

class StepOverCmd : public XDebugCommand {
public:
  StepOverCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~StepOverCmd() {}

  // Respond on next break
  bool shouldRespond() const override { return false; }

  bool isValidInStatus(Status status) const override {
    return
      status == Status::STARTING ||
      status == Status::STOPPING ||
      status == Status::BREAK;
  }

  bool handleImpl(xdebug_xml_node& xml) const override {
    phpDebuggerNext();
    return true;
  }
};

////////////////////////////////////////////////////////////////////////////////
// stop -i #
// TODO(#4489053) Implement

class StopCmd : public XDebugCommand {
public:
  StopCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~StopCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
  }
};

////////////////////////////////////////////////////////////////////////////////
// detach -i #
// TODO(#4489053) Implement

class DetachCmd : public XDebugCommand {
public:
  DetachCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~DetachCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
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
  XDebugBreakpoint m_breakpoint;
};

////////////////////////////////////////////////////////////////////////////////
// breakpoint_get -i #
// TODO(#4489053) Implement

class BreakpointGetCmd : public XDebugCommand {
public:
  BreakpointGetCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~BreakpointGetCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
  }
};

////////////////////////////////////////////////////////////////////////////////
// breakpoint_update -i #
// TODO(#4489053) Implement

class BreakpointUpdateCmd : public XDebugCommand {
public:
  BreakpointUpdateCmd(XDebugServer& server,
                      const String& cmd,
                      const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~BreakpointUpdateCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
  }
};

////////////////////////////////////////////////////////////////////////////////
// breakpoint_remove -i #
// TODO(#4489053) Implement

class BreakpointRemoveCmd : public XDebugCommand {
public:
  BreakpointRemoveCmd(XDebugServer& server,
                      const String& cmd,
                      const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~BreakpointRemoveCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
  }
};

////////////////////////////////////////////////////////////////////////////////
// stack_depth -i #
// TODO(#4489053) Implement

class StackDepthCmd : public XDebugCommand {
public:
  StackDepthCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~StackDepthCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
  }
};

////////////////////////////////////////////////////////////////////////////////
// stack_get -i #
// TODO(#4489053) Implement

class StackGetCmd : public XDebugCommand {
public:
  StackGetCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~StackGetCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
  }
};

////////////////////////////////////////////////////////////////////////////////
// context_names -i #
// TODO(#4489053) Implement

class ContextNamesCmd : public XDebugCommand {
public:
  ContextNamesCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~ContextNamesCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
  }
};

////////////////////////////////////////////////////////////////////////////////
// context_get -i #
// TODO(#4489053) Implement

class ContextGetCmd : public XDebugCommand {
public:
  ContextGetCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~ContextGetCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
  }
};

////////////////////////////////////////////////////////////////////////////////
// property_get -i #
// TODO(#4489053) Implement

class PropertyGetCmd : public XDebugCommand {
public:
  PropertyGetCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~PropertyGetCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
  }
};

////////////////////////////////////////////////////////////////////////////////
// property_set -i #
// TODO(#4489053) Implement

class PropertySetCmd : public XDebugCommand {
public:
  PropertySetCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~PropertySetCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
  }
};

////////////////////////////////////////////////////////////////////////////////
// property_value -i #
// TODO(#4489053) Implement

class PropertyValueCmd : public XDebugCommand {
public:
  PropertyValueCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~PropertyValueCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
  }
};

////////////////////////////////////////////////////////////////////////////////
// source -i #
// TODO(#4489053) Implement

class SourceCmd : public XDebugCommand {
public:
  SourceCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~SourceCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
  }
};

////////////////////////////////////////////////////////////////////////////////
// stdout -i #
// TODO(#4489053) Implement

class StdoutCmd : public XDebugCommand {
public:
  StdoutCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~StdoutCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
  }
};

////////////////////////////////////////////////////////////////////////////////
// stderr -i #
// TODO(#4489053) Implement

class StderrCmd : public XDebugCommand {
public:
  StderrCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~StderrCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
  }
};

////////////////////////////////////////////////////////////////////////////////
// eval -i #
// TODO(#4489053) Implement

class EvalCmd : public XDebugCommand {
public:
  EvalCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~EvalCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
  }
};

////////////////////////////////////////////////////////////////////////////////
// xcmd_profiler_name_get -i #
// TODO(#4489053) Implement

class ProfilerNameGetCmd : public XDebugCommand {
public:
  ProfilerNameGetCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~ProfilerNameGetCmd() {}

  bool handleImpl(xdebug_xml_node& xml) const override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
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
  // Match will be set true once there is a match.
  bool match = false;
  string cmd_cpp = cmdStr.toCppString();

  // Check each command
  XDebugCommand* cmd;
  #define COMMAND(name, className)                                             \
    if (!match && cmd_cpp == name) {                                           \
      cmd = new className(server, cmdStr, args);                               \
      match = true;                                                            \
    }
  COMMANDS
  #undef COMMAND

  // php5 xdebug throws an unimplemented error when no valid match is found
  if (!match) {
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
