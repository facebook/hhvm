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
#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_var.h"

#include "hphp/compiler/builtin_symbols.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/ext/ext_file.h"
#include "hphp/runtime/ext/std/ext_std_misc.h"
#include "hphp/runtime/ext/url/ext_url.h"
#include "hphp/runtime/vm/runtime.h"
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
  COMMAND("breakpoint_list", BreakpointListCmd)                                \
  COMMAND("breakpoint_update", BreakpointUpdateCmd)                            \
  COMMAND("breakpoint_remove", BreakpointRemoveCmd)                            \
  COMMAND("stack_depth", StackDepthCmd)                                        \
  COMMAND("stack_get", StackGetCmd)                                            \
  COMMAND("context_names", ContextNamesCmd)                                    \
  COMMAND("context_get", ContextGetCmd)                                        \
  COMMAND("typemap_get", TypemapGetCmd)                                        \
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
// Helpers

// These are used a lot, prevent unnecessary verbosity
typedef XDebugServer::Status Status;
typedef XDebugServer::Reason Reason;

// Compiles the encoded base64 string expression and returns the corresponding
// unit. Throws XDebugServer::ERROR_EVALUATING_CODE on failure.
//
// Note that php5 xdebug claims non-expressions are allowed in their eval code,
// but the implementation calls zend_eval_string which wraps EXPR in
// "return EXPR;"
static Unit* xdebug_compile(const String& expr) {
  // Decode the string then make it return
  StringBuffer buf;
  String decoded = StringUtil::Base64Decode(expr);
  buf.printf("<?php return %s;", decoded.data());
  String evalString = buf.detach();

  // Try to compile the string, don't JIT the code since it is not likely to
  // be used often
  Unit* unit = compile_string(evalString.data(), evalString.size());
  if (unit == nullptr) {
    throw XDebugServer::ERROR_EVALUATING_CODE;
  }
  unit->setInterpretOnly();
  return unit;
}

// Helper for the breakpoint commands that returns an xml node containing
// breakpoint information
static xdebug_xml_node* breakpoint_xml_node(int id,
                                            const XDebugBreakpoint& bp) {
  // Initialize the xml node
  xdebug_xml_node* xml = xdebug_xml_node_init("breakpoint");
  xdebug_xml_add_attribute(xml, "id", id);

  // It looks like php5 xdebug used to consider "temporary" as a state. It's
  // changed everywhere except here in xdebug's code. An obvious improvement
  // would to output an extra "temporary" attribute, but for now this logic
  // just follows xdebug
  if (bp.temporary) {
    xdebug_xml_add_attribute(xml, "state", "temporary");
  } else if (bp.enabled) {
    xdebug_xml_add_attribute(xml, "state", "enabled");
  } else {
    xdebug_xml_add_attribute(xml, "state", "disabled");
  }

  // Add the hit condition and count
  switch (bp.hitCondition) {
    case XDebugBreakpoint::HitCondition::GREATER_OR_EQUAL:
      xdebug_xml_add_attribute(xml, "hit_condition", ">=");
      break;
    case XDebugBreakpoint::HitCondition::EQUAL:
      xdebug_xml_add_attribute(xml, "hit_condition", "==");
      break;
    case XDebugBreakpoint::HitCondition::MULTIPLE:
      xdebug_xml_add_attribute(xml, "hit_condition", "%");
      break;
  }
  xdebug_xml_add_attribute(xml, "hit_count", bp.hitCount);

  // Add type specific info
  switch (bp.type) {
    // Line breakpoints add a file, line, and possibly a condition
    case XDebugBreakpoint::Type::LINE:
      xdebug_xml_add_attribute(xml, "type", "line");
      xdebug_xml_add_attribute_ex(xml, "filename",
                                  XDebugUtils::pathToUrl(bp.fileName.data()),
                                  0, 1);
      xdebug_xml_add_attribute(xml, "lineno", bp.line);

      // Add the condition. cast is due to xml api
      if (bp.conditionUnit != nullptr) {
        xdebug_xml_node* expr_xml = xdebug_xml_node_init("expression");
        xdebug_xml_add_text(expr_xml,
                            const_cast<char*>(bp.condition.data()), 0);
        xdebug_xml_add_child(xml, expr_xml);
      }
      break;
    // Exception breakpoints just add the type
    case XDebugBreakpoint::Type::EXCEPTION:
      xdebug_xml_add_attribute(xml, "type", "exception");
      break;
    // Call breakpoints add function + class (optionally)
    case XDebugBreakpoint::Type::CALL:
      xdebug_xml_add_attribute(xml, "type", "call");
      xdebug_xml_add_attribute(xml, "function", bp.funcName.data());
      if (!bp.className.isNull()) {
        xdebug_xml_add_attribute_dup(xml, "class",
                                     bp.className.toString().data());
      }
      break;
    // Return breakpoints add function + class (optionally)
    case XDebugBreakpoint::Type::RETURN:
      xdebug_xml_add_attribute(xml, "type", "return");
      xdebug_xml_add_attribute(xml, "function", bp.funcName.data());
      if (!bp.className.isNull()) {
        xdebug_xml_add_attribute_dup(xml, "class",
                                     bp.className.toString().data());
      }
      break;
  }
  return xml;
}

// Helper that adds the property with the given name, looking it up from the
// given frame, to the given xml node, using the given exporter. If the symbol
// is invalid, an error code is thrown.
void xdebug_add_property_xml(xdebug_xml_node& root,
                             const String& name,
                             int depth,
                             XDebugVarType type,
                             XDebugExporter exporter) {
  String decoded = StringUtil::Base64Decode(name);

  ActRec* fp = g_context->getStackFrame();
  Variant val = xdebug_get_php_symbol(fp, decoded.get());

  if (!val.isInitialized()) {
    throw XDebugServer::ERROR_PROPERTY_NON_EXISTANT;
  }

  // Success, add the property xml
  xdebug_xml_node* xml =
    xdebug_get_value_xml_node(decoded.data(), val, type, exporter);
  xdebug_xml_add_child(&root, xml);
}

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

  void handleImpl(xdebug_xml_node& xml) override {
    m_server.addStatus(xml);
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

  bool isValidInStatus(Status status) const override { return true; }

  void handleImpl(xdebug_xml_node& xml) override {
    // Set to true once we have a match. Const cast is needed due to xdebug
    // xml api.
    bool match = false;
    xdebug_xml_add_attribute(&xml, "feature_name", m_feature.c_str());

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

  void handleImpl(xdebug_xml_node& xml) override {
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
    xdebug_xml_add_attribute(&xml, "feature", m_feature.c_str());
    xdebug_xml_add_attribute(&xml, "success", "1");
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
  bool shouldContinue() const override { return true; }

  bool isValidInStatus(Status status) const override {
    return
      status == Status::STARTING ||
      status == Status::STOPPING ||
      status == Status::BREAK;
  }

  void handleImpl(xdebug_xml_node& xml) override {
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
  bool shouldContinue() const override { return true; }

  bool isValidInStatus(Status status) const override {
    return
      status == Status::STARTING ||
      status == Status::STOPPING ||
      status == Status::BREAK;
  }

  void handleImpl(xdebug_xml_node& xml) override {
    phpDebuggerStepIn();
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
  bool shouldContinue() const override { return true; }

  bool isValidInStatus(Status status) const override {
    return
      status == Status::STARTING ||
      status == Status::STOPPING ||
      status == Status::BREAK;
  }

  void handleImpl(xdebug_xml_node& xml) override {
    phpDebuggerStepOut();
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
  bool shouldContinue() const override { return true; }

  bool isValidInStatus(Status status) const override {
    return
      status == Status::STARTING ||
      status == Status::STOPPING ||
      status == Status::BREAK;
  }

  void handleImpl(xdebug_xml_node& xml) override {
    phpDebuggerNext();
  }
};

////////////////////////////////////////////////////////////////////////////////
// stop -i #
// Stops execution of the script by exiting

class StopCmd : public XDebugCommand {
public:
  StopCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~StopCmd() {}

  bool isValidInStatus(Status status) const override { return true; }

  void handleImpl(xdebug_xml_node& xml) override {
    m_server.setStatus(Status::STOPPED, Reason::OK);

    // We need to throw an exception, so this needs to be sent manually
    m_server.addStatus(xml);
    m_server.sendMessage(xml);
    throw ExitException(0);
  }
};

////////////////////////////////////////////////////////////////////////////////
// detach -i #
// Detaches the xdebug server. In php5 xdebug this just means the user cannot
// input commands.

class DetachCmd : public XDebugCommand {
public:
  DetachCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~DetachCmd() {}

  void handleImpl(xdebug_xml_node& xml) override {
    m_server.setStatus(Status::DETACHED, Reason::OK);
    m_server.addStatus(xml);
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
  s_MOD("%"),
  s_user("user");

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

      // Create the condition unit if a condition string was supplied
      if (!args['-'].isNull()) {
        String condition = args['-'].toString();
        bp.condition = condition;
        bp.conditionUnit = xdebug_compile(condition);
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

  void handleImpl(xdebug_xml_node& xml) override {
    // Add the breakpoint, write out the id
    int id = XDEBUG_ADD_BREAKPOINT(m_breakpoint);
    xdebug_xml_add_attribute(&xml, "id", id);

    // Add the breakpoint state
    if (m_breakpoint.enabled) {
      xdebug_xml_add_attribute(&xml, "state", "enabled");
    } else {
      xdebug_xml_add_attribute(&xml, "state", "disabled");
    }
  }

private:
  XDebugBreakpoint m_breakpoint;
};

////////////////////////////////////////////////////////////////////////////////
// breakpoint_get -i # -d ID
// Returns information about the breakpoint with the given id

class BreakpointGetCmd : public XDebugCommand {
public:
  BreakpointGetCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {
    // Breakpoint id must be provided
    if (args['d'].isNull()) {
      throw XDebugServer::ERROR_INVALID_ARGS;
    }
    m_id = strtol(args['d'].toString().data(), nullptr, 10);
  }

  ~BreakpointGetCmd() {}

  void handleImpl(xdebug_xml_node& xml) override {
    const XDebugBreakpoint* bp = XDEBUG_GET_BREAKPOINT(m_id);
    if (bp == nullptr) {
      throw XDebugServer::ERROR_NO_SUCH_BREAKPOINT;
    }
    xdebug_xml_add_child(&xml, breakpoint_xml_node(m_id, *bp));
  }

private:
  int m_id;
};

////////////////////////////////////////////////////////////////////////////////
// breakpoint_list -i #
// Returns all the registered breakpoints

class BreakpointListCmd : public XDebugCommand {
public:
  BreakpointListCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~BreakpointListCmd() {}

  bool isValidInStatus(Status status) const override { return true; }

  void handleImpl(xdebug_xml_node& xml) override {
    for (auto iter = XDEBUG_BREAKPOINTS.begin();
         iter != XDEBUG_BREAKPOINTS.end(); ++iter) {
      int id = iter->first;
      const XDebugBreakpoint& bp = iter->second;
      xdebug_xml_add_child(&xml, breakpoint_xml_node(id, bp));
    }
  }
};

////////////////////////////////////////////////////////////////////////////////
// breakpoint_update -i # -d id [-s STATE] [-n LINE] [-h HIT_VALUE]
//                              [-o HIT_CONDITION]
// Updates the breakpoint with the given id using the given arguments

class BreakpointUpdateCmd : public XDebugCommand {
public:
  BreakpointUpdateCmd(XDebugServer& server,
                      const String& cmd,
                      const Array& args)
    : XDebugCommand(server, cmd, args) {
    // Breakpoint id must be provided
    if (args['d'].isNull()) {
      throw XDebugServer::ERROR_INVALID_ARGS;
    }
    m_id = strtol(args['d'].toString().data(), nullptr, 10);

    // Grab the new state if it was passed
    if (!args['s'].isNull()) {
      String state = args['s'].toString();
      if (state == s_ENABLED) {
        m_enabled = true;
      } else if (state == s_DISABLED) {
        m_enabled = false;
      } else {
        throw XDebugServer::ERROR_INVALID_ARGS;
      }
      m_hasEnabled = true;
    }

    // Grab the new line if it was passed
    if (!args['n'].isNull()) {
      m_hasLine = true;
      m_line = strtol(args['n'].toString().data(), nullptr, 10);
    }

    // Grab the new hit value if it was passed
    if (!args['h'].isNull()) {
      m_hasHitValue = true;
      m_hitValue = strtol(args['h'].toString().data(), nullptr, 10);
    }

    // Grab the hit condition if it was passed
    if (!args['o'].isNull()) {
      String condition = args['o'].toString();
      if (condition == s_GREATER_OR_EQUAL) {
        m_hitCondition = XDebugBreakpoint::HitCondition::GREATER_OR_EQUAL;
      } else if (condition == s_EQUAL) {
        m_hitCondition = XDebugBreakpoint::HitCondition::EQUAL;
      } else if (condition == s_MOD) {
        m_hitCondition = XDebugBreakpoint::HitCondition::MULTIPLE;
      } else {
        throw XDebugServer::ERROR_INVALID_ARGS;
      }
      m_hasHitCondition = true;
    }
  }

  ~BreakpointUpdateCmd() {}

  void handleImpl(xdebug_xml_node& xml) override {
    // Need to grab the breakpoint to send back the breakpoint info
    const XDebugBreakpoint* bp = XDEBUG_GET_BREAKPOINT(m_id);
    if (bp == nullptr) {
      throw XDebugServer::ERROR_NO_SUCH_BREAKPOINT;
    }

    // If any of the updates fails an error is thrown
    if (m_hasEnabled &&
        !s_xdebug_breakpoints->updateBreakpointState(m_id, m_enabled)) {
      throw XDebugServer::ERROR_BREAKPOINT_INVALID;
    }
    if (m_hasHitCondition &&
        !s_xdebug_breakpoints->updateBreakpointHitCondition(m_id,
                                                            m_hitCondition)) {
      throw XDebugServer::ERROR_BREAKPOINT_INVALID;
    }
    if (m_hasHitValue &&
        !s_xdebug_breakpoints->updateBreakpointHitValue(m_id, m_hitValue)) {
      throw XDebugServer::ERROR_BREAKPOINT_INVALID;
    }
    if (m_hasLine &&
        !s_xdebug_breakpoints->updateBreakpointLine(m_id, m_line)) {
      throw XDebugServer::ERROR_BREAKPOINT_INVALID;
    }

    // Add the breakpoint info. php5 xdebug does this, the spec does
    // not specify this.
    xdebug_xml_node* node = breakpoint_xml_node(m_id, *bp);
    xdebug_xml_add_child(&xml, node);
  }

private:
  int m_id;

  // Options that can be passed
  bool m_hasEnabled = false;
  bool m_hasLine = false;
  bool m_hasHitValue = false;
  bool m_hasHitCondition = false;

  // Valid if the corresponding boolean is true
  bool m_enabled;
  int m_line;
  int m_hitValue;
  XDebugBreakpoint::HitCondition m_hitCondition;
};

////////////////////////////////////////////////////////////////////////////////
// breakpoint_remove -i # -d ID
// Removes the breakpoint with the given id

class BreakpointRemoveCmd : public XDebugCommand {
public:
  BreakpointRemoveCmd(XDebugServer& server,
                      const String& cmd,
                      const Array& args)
    : XDebugCommand(server, cmd, args) {
    // Breakpoint id must be provided
    if (args['d'].isNull()) {
      throw XDebugServer::ERROR_INVALID_ARGS;
    }
    m_id = strtol(args['d'].toString().data(), nullptr, 10);
  }

  ~BreakpointRemoveCmd() {}

  void handleImpl(xdebug_xml_node& xml) override {
    const XDebugBreakpoint* bp = XDEBUG_GET_BREAKPOINT(m_id);
    if (bp == nullptr) {
      throw XDebugServer::ERROR_NO_SUCH_BREAKPOINT;
    }

    // spec doesn't specify this, but php5 xdebug sends back breakpoint info
    xdebug_xml_node* node = breakpoint_xml_node(m_id, *bp);
    xdebug_xml_add_child(&xml, node);

    // The breakpoint is deleted, so this has to be last
    XDEBUG_REMOVE_BREAKPOINT(m_id);
  }

private:
  int m_id;
};

////////////////////////////////////////////////////////////////////////////////
// stack_depth -i #
// Returns the current stack depth

class StackDepthCmd : public XDebugCommand {
public:
  StackDepthCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~StackDepthCmd() {}

  void handleImpl(xdebug_xml_node& xml) override {
    int depth = XDebugUtils::stackDepth();
    xdebug_xml_add_attribute(&xml, "depth", depth);
  }
};

////////////////////////////////////////////////////////////////////////////////
// stack_get -i # [-d DEPTH]
// Returns the stack at the given depth, or the entire stack if no depth is
// provided

const static StaticString s_FILE("file");

class StackGetCmd : public XDebugCommand {
public:
  StackGetCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {
    // Grab the optional depth argument
    if (!args['d'].isNull()) {
      m_clientDepth = strtol(args['d'].toString().data(), nullptr, 10);
      if (m_clientDepth < 0 || m_clientDepth > XDebugUtils::stackDepth()) {
        throw XDebugServer::ERROR_STACK_DEPTH_INVALID;
      }
    }
  }

  ~StackGetCmd() {}

  bool isValidInStatus(Status status) const override { return true; }

  void handleImpl(xdebug_xml_node& xml) override {
    // Iterate up the stack. We need to keep track of both the frame actrec and
    // our current depth in case the client passed us a depth
    int depth = 0;
    const ActRec* last_fp = nullptr;
    for (const ActRec* fp = g_context->getStackFrame();
         fp != nullptr && (m_clientDepth == -1 || depth <= m_clientDepth);
         fp = g_context->getPrevVMState(fp), depth++) {
      // If a depth was provided, we're only interested in that depth
      if (m_clientDepth < 0 || depth == m_clientDepth) {
        xdebug_xml_node* frame = getFrame(fp, last_fp, depth);
        xdebug_xml_add_child(&xml, frame);
      }
      last_fp = fp;
    }
  }

private:
  // Returns the xml node for the given stack frame. The lastFp, if provided,
  // is the frame "below" the passed fp.
  xdebug_xml_node* getFrame(const ActRec* fp, const ActRec* lastFp, int level) {
    Offset call_offset;
    const Func* func = fp->func();
    const ActRec* prev_fp = g_context->getPrevVMState(fp, &call_offset);

    // Compute the function name. php5 xdebug includes names for each type of
    // include, we don't have access to that
    const char* func_name = func->isPseudoMain() ?
      (prev_fp == nullptr ? "{main}" : "include") : func->fullName()->data();

    // Create the frame node
    xdebug_xml_node* node = xdebug_xml_node_init("stack");
    xdebug_xml_add_attribute(node, "where", func_name);
    xdebug_xml_add_attribute(node, "level", level);
    xdebug_xml_add_attribute(node, "type", "file");

    // Grab the callsite. php5 xdebug returns the current line/file for the
    // top level.
    String call_file; int call_line;
    if (prev_fp != nullptr) {
      const Unit* prev_unit = prev_fp->func()->unit();
      call_file = String(prev_unit->filepath()->data(), CopyString);
      call_line = prev_unit->getLineNumber(call_offset);
    } else if (lastFp != nullptr) {
      call_file = String(func->filename()->data(), CopyString);
      call_line = func->unit()->getLineNumber(func->base() + lastFp->m_soff);
    } else {
      // Currently at the top level
      call_file = String(func->filename()->data(), CopyString);
      call_line = g_context->getLine();
    }
    call_file = XDebugUtils::pathToUrl(call_file); // file output format

    // Add the call file/line. Duplications are necessary due to xml api
    xdebug_xml_add_attribute_dup(node, "filename", call_file.data());
    xdebug_xml_add_attribute(node, "lineno", call_line);
    return node;
  }

private:
  int m_clientDepth = -1; // If >= 0, depth of the stack frame the client wants
};

////////////////////////////////////////////////////////////////////////////////
// context_names -i # [-d DEPTH]
// Returns the names of the currently available contexts at the given depth

// This a really weird set of contexts. But they match php5 xdebug. Adding
// contexts such as class would make sense
enum class XDebugContext : int {
  LOCAL = 1,
  SUPERGLOBALS = 2,
  USER_CONSTANTS = 3
};

class ContextNamesCmd : public XDebugCommand {
public:
  ContextNamesCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~ContextNamesCmd() {}

  bool isValidInStatus(Status status) const override { return true; }

  void handleImpl(xdebug_xml_node& xml) override {
    xdebug_xml_node* child = xdebug_xml_node_init("context");
    xdebug_xml_add_attribute(child, "name", "Locals");
    xdebug_xml_add_attribute(child, "id",
                             static_cast<int>(XDebugContext::LOCAL));
    xdebug_xml_add_child(&xml, child);

    child = xdebug_xml_node_init("context");
    xdebug_xml_add_attribute(child, "name", "Superglobals");
    xdebug_xml_add_attribute(child, "id",
                             static_cast<int>(XDebugContext::SUPERGLOBALS));
    xdebug_xml_add_child(&xml, child);

    child = xdebug_xml_node_init("context");
    xdebug_xml_add_attribute(child, "name", "User defined constants");
    xdebug_xml_add_attribute(child, "id",
                             static_cast<int>(XDebugContext::USER_CONSTANTS));
    xdebug_xml_add_child(&xml, child);
  }
};

////////////////////////////////////////////////////////////////////////////////
// context_get -i # [-d DEPTH] [-c CONTEXT]

class ContextGetCmd : public XDebugCommand {
public:
  ContextGetCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {
    // Grab the context if it was passed
    if (!args['c'].isNull()) {
      int context = args['c'].toInt32();
      switch (context) {
        case static_cast<int>(XDebugContext::LOCAL):
        case static_cast<int>(XDebugContext::SUPERGLOBALS):
        case static_cast<int>(XDebugContext::USER_CONSTANTS):
          m_context = static_cast<XDebugContext>(context);
          break;
        default:
          throw XDebugServer::ERROR_INVALID_ARGS;
      }
    }

    // Grab the depth if it was provided
    if (!args['d'].isNull()) {
      m_depth = strtol(args['d'].toString().data(), nullptr, 10);
    } else {
      m_depth = 0;
    }
  }

  ~ContextGetCmd() {}

  void handleImpl(xdebug_xml_node& xml) override {
    // Setup the variable exporter
    // TODO(#4489053) This may or may not match php5 xdebug as far as the format
    XDebugExporter exporter;
    exporter.max_depth = m_server.m_maxDepth;
    exporter.max_children = m_server.m_maxChildren;
    exporter.max_data = m_server.m_maxData;
    exporter.page = 0;

    // Grab from the requested context
    switch (m_context) {
      case XDebugContext::SUPERGLOBALS: {
        Array globals = php_globals_as_array();
        for (ArrayIter iter(globals); iter; ++iter) {
          if (!BuiltinSymbols::IsSuperGlobal(iter.first().toString().data())) {
            continue;
          }

          xdebug_xml_node* node =
            xdebug_get_value_xml_node(iter.first().toString().data(),
                                      iter.second(), XDebugVarType::Normal,
                                      exporter);
          xdebug_xml_add_child(&xml, node);
        }
        break;
      }
      case XDebugContext::USER_CONSTANTS: {
        Array allConstants = lookupDefinedConstants(true);

        Array constants = allConstants[s_user].toArray();
        for (ArrayIter iter(constants); iter; ++iter) {
          xdebug_xml_node* node =
            xdebug_get_value_xml_node(iter.first().toString().data(),
                                      iter.second(), XDebugVarType::Normal,
                                      exporter);
          xdebug_xml_add_child(&xml, node);
        }
        break;
      }
      // TODO(#4489053) This may or may not match php5 xdebug as far as which
      // variables are in scope
      case XDebugContext::LOCAL: {
        // Grab the in scope variables
        VarEnv* var_env = g_context->getVarEnv(m_depth);
        if (var_env == nullptr) {
          throw XDebugServer::ERROR_INVALID_ARGS;
        }

        // Add each variable
        Array vars = var_env->getDefinedVariables();
        for (ArrayIter iter(vars); iter; ++iter) {
          xdebug_xml_node* node =
            xdebug_get_value_xml_node(iter.first().toString().data(),
                                      iter.second(), XDebugVarType::Normal,
                                      exporter);
          xdebug_xml_add_child(&xml, node);
        }
        break;
      }
    }
  }

private:
  XDebugContext m_context = XDebugContext::LOCAL;
  int m_depth = 0;
};

////////////////////////////////////////////////////////////////////////////////
// typemap_get -i #
// Returns the types supported as well their corresponding xml schema

// Taken from php5 xdebug
// Fields are common name, lang name, and schema
#define XDEBUG_TYPES_COUNT 8
static const char* s_TYPEMAP[XDEBUG_TYPES_COUNT][3] = {
  {"bool",     "bool",     "xsd:boolean"},
  {"int",      "int",      "xsd:decimal"},
  {"float",    "float",    "xsd:double"},
  {"string",   "string",   "xsd:string"},
  {"null",     "null",     nullptr},
  {"hash",     "array",    nullptr},
  {"object",   "object",   nullptr},
  {"resource", "resource", nullptr}
};

class TypemapGetCmd : public XDebugCommand {
public:
  TypemapGetCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~TypemapGetCmd() {}

  bool isValidInStatus(Status status) const override { return true; }

  void handleImpl(xdebug_xml_node& xml) override {
    // Add the schema
    xdebug_xml_add_attribute(&xml, "xmlns:xsi",
                             "http://www.w3.org/2001/XMLSchema-instance");
    xdebug_xml_add_attribute(&xml, "xmlns:xsd",
                             "http://www.w3.org/2001/XMLSchema");

    // Add the types. Casts are necessary due to xml api
    for (int i = 0; i < XDEBUG_TYPES_COUNT; i++) {
      xdebug_xml_node* type = xdebug_xml_node_init("map");
      xdebug_xml_add_attribute(type, "name", s_TYPEMAP[i][1]);
      xdebug_xml_add_attribute(type, "type", s_TYPEMAP[i][0]);
      if (s_TYPEMAP[i][2]) {
        xdebug_xml_add_attribute(type, "xsi:type", s_TYPEMAP[i][2]);
      }
      xdebug_xml_add_child(&xml, type);
    }
  }
};


////////////////////////////////////////////////////////////////////////////////
// property_get -i # -n LONGNAME [-d DEPTH] [-c CONTEXT] [-m MAX_DATA] [-p PAGE]
// Note that the spec mentioned a 'k' and 'a', php5 xdebug does not support it
// Gets the specified property value

class PropertyGetCmd : public XDebugCommand {
public:
  PropertyGetCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {
    // A name is required
    if (args['n'].isNull()) {
      throw XDebugServer::ERROR_INVALID_ARGS;
    }
    m_name = args['n'].toString();

    // Grab the context if it provided
    if (!args['c'].isNull()) {
      int context = strtol(args['c'].toString().data(), nullptr, 10);
      switch (context) {
        case static_cast<int>(XDebugContext::LOCAL):
        case static_cast<int>(XDebugContext::SUPERGLOBALS):
        case static_cast<int>(XDebugContext::USER_CONSTANTS):
          m_context = static_cast<XDebugContext>(context);
          break;
        default:
          throw XDebugServer::ERROR_INVALID_ARGS;
      }
    }

    // Grab the depth if it is provided
    if (!args['d'].isNull()) {
      m_depth = strtol(args['d'].toString().data(), nullptr, 10);
    }

    // Grab the page if it was provided
    if (!args['p'].isNull()) {
      m_page = strtol(args['p'].toString().data(), nullptr, 10);
    }

    // Grab the max data if it was provided
    if (!args['m'].isNull()) {
      m_maxData= strtol(args['m'].toString().data(), nullptr, 10);
    }
  }

  ~PropertyGetCmd() {}

  void handleImpl(xdebug_xml_node& xml) override {
    // Get the correct stack frame
    ActRec* fp = g_context->getStackFrame();
    for (int depth = 0; fp != nullptr && depth < m_depth;
         depth++, fp = g_context->getPrevVMState(fp)) {}

    // If we don't have an actrec, the stack depth was invalid
    if (fp == nullptr) {
      throw XDebugServer::ERROR_STACK_DEPTH_INVALID;
    }

    // Setup the variable exporter
    XDebugExporter exporter;
    exporter.max_depth = m_maxDepth;
    exporter.max_children = m_maxChildren;
    exporter.max_data = m_maxData;
    exporter.page = m_page;

    switch (m_context) {
      // Globals and superglobals can be fetched via xdebug_get_php_symbol
      case XDebugContext::LOCAL:
      case XDebugContext::SUPERGLOBALS:
        xdebug_add_property_xml(xml, m_name, m_depth,
                                XDebugVarType::Normal, exporter);
        break;
      // Grab the the constant, f_constant throws a warning on failure so
      // we ensure it's defined before grabbing it
      case XDebugContext::USER_CONSTANTS: {
        if (!f_defined(m_name)) {
          throw XDebugServer::ERROR_PROPERTY_NON_EXISTANT;
        }

        // php5 xdebug adds "constant" facet, but this is not in the spec
        xdebug_xml_node* node =
          xdebug_get_value_xml_node(m_name.data(), f_constant(m_name),
                                    XDebugVarType::Constant, exporter);
        xdebug_xml_add_attribute(node, "facet", "constant");
        xdebug_xml_add_child(&xml, node);
        break;
      }
    }
}

private:
  String m_name;
  XDebugContext m_context = XDebugContext::LOCAL;
  int m_depth = 0; // desired stack depth
  int m_maxDepth = m_server.m_maxDepth; // max property depth
  int m_maxData = m_server.m_maxData;
  int m_maxChildren = m_server.m_maxChildren;
  int m_page = 0;
};

////////////////////////////////////////////////////////////////////////////////
// property_set -i #
// TODO(#4489053) Implement

class PropertySetCmd : public XDebugCommand {
public:
  PropertySetCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~PropertySetCmd() {}

  void handleImpl(xdebug_xml_node& xml) override {
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

  void handleImpl(xdebug_xml_node& xml) override {
    throw XDebugServer::ERROR_UNIMPLEMENTED;
  }
};

////////////////////////////////////////////////////////////////////////////////
// source -i # [-f FILE] [-b BEGIN] [-e END]
// Grabs the given source file starting at the optionally given begin and end
// lines.

class SourceCmd : public XDebugCommand {
public:
  SourceCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {
    // Either grab the passed filename or get the current one
    if (args['f'].isNull()) {
      StringData* filename_data = g_context->getContainingFileName();
      if (filename_data == staticEmptyString()) {
        throw XDebugServer::ERROR_STACK_DEPTH_INVALID;
      }
      m_filename = String(filename_data);
    } else {
      m_filename = XDebugUtils::pathFromUrl(args['f'].toString());
    }
    m_filename = File::TranslatePath(m_filename); // canonicolize path


    // Grab and 0-index the begin line
    if (!args['b'].isNull()) {
      m_beginLine = strtol(args['b'].toString().data(), nullptr, 10) - 1;
    }

    // Grab and 0-index the end line
    if (!args['e'].isNull()) {
      m_endLine = strtol(args['e'].toString().data(), nullptr, 10) - 1;
    }
  }

  ~SourceCmd() {}

  void handleImpl(xdebug_xml_node& xml) override {
    // Grab the file as an array
    Variant file = f_file(m_filename);
    if (!file.isArray()) {
      throw XDebugServer::ERROR_CANT_OPEN_FILE;
    }
    Array source = file.toArray();

    // Compute the begin/end line
    if (m_beginLine < 0) {
      m_beginLine = 0;
    }
    if (m_endLine < 0) {
      m_endLine = source.size() - 1;
    }

    // Compute the source string. The initial size is arbitrary, we just guess
    // 80 chracters per line
    StringBuffer buf((m_endLine - m_beginLine) * 80);
    ArrayIter iter(source); iter.setPos(m_beginLine);
    for (int i = m_beginLine; i <= m_endLine && iter; i++, ++iter) {
      buf.append(iter.second());
    }
    m_source = buf.detach(); // To keep alive as long as command is alive

    // Attach the source, const cast is due to xml interface
    xdebug_xml_add_text_ex(&xml, const_cast<char*>(m_source.data()),
                           m_source.size(), 0, 1);
  }

private:
  String m_filename;
  String m_source;
  int m_beginLine = 0;
  int m_endLine = -1;
};

////////////////////////////////////////////////////////////////////////////////
// stdout -i # -c 0|1|2
// Redirect or copy stdout to the client

// Helper called on stdout write when we have redirected it with the stdout
// command. Once installed, this continues until it is either explicitly
// uninstalled, or if there is no longer an xdebug server.
static void onStdoutWrite(const char* bytes, int len, void* copy) {
  if (XDEBUG_GLOBAL(Server) == nullptr) {
    g_context->setStdout(nullptr, nullptr);
  }
  XDEBUG_GLOBAL(Server)->sendStream("stdout", bytes, len);

  // If copy is true, we also copy the data to stdout
  if ((bool) copy) {
    write(fileno(stdout), bytes, len);
  }
}

class StdoutCmd : public XDebugCommand {
public:
  StdoutCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {
    // "c" must be provided
    if (args['c'].isNull()) {
      throw XDebugServer::ERROR_INVALID_ARGS;
    }

    // Only several types of modes are allowed
    int mode = strtol(args['c'].toString().data(), nullptr, 10);
    switch (mode) {
      case MODE_DISABLE:
      case MODE_COPY:
      case MODE_REDIRECT:
        m_mode = static_cast<Mode>(mode);
        break;
      default:
        throw XDebugServer::ERROR_INVALID_ARGS;
    }
  }

  ~StdoutCmd() {}

  void handleImpl(xdebug_xml_node& xml) override {
    switch (m_mode) {
      case MODE_DISABLE:
        g_context->setStdout(nullptr, nullptr);
        break;
      case MODE_COPY:
        g_context->setStdout(onStdoutWrite, (void*) true);
        break;
      case MODE_REDIRECT:
        g_context->setStdout(onStdoutWrite, (void*) false);
        break;
      default:
        throw Exception("Invalid mode type");
    }

    xdebug_xml_add_attribute(&xml, "success", "1");
  }

private:
  enum Mode {
    MODE_DISABLE = 0,
    MODE_COPY = 1,
    MODE_REDIRECT = 2
  };
  Mode m_mode;
};

////////////////////////////////////////////////////////////////////////////////
// stderr -i #
// This "required" dbgp-core feature is not implemented by php5 xdebug :)

class StderrCmd : public XDebugCommand {
public:
  StderrCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~StderrCmd() {}

  void handleImpl(xdebug_xml_node& xml) override {
    xdebug_xml_add_attribute(&xml, "success", "0");
  }
};

////////////////////////////////////////////////////////////////////////////////
// eval -i # [-p PAGE] -- DATA
// Evaluate a string within the given exeuction context.

class EvalCmd : public XDebugCommand {
public:
  EvalCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {
    // An evaluation string must be provided
    if (args['-'].isNull()) {
      throw XDebugServer::ERROR_INVALID_ARGS;
    }
    m_evalUnit = xdebug_compile(args['-'].toString());

    // A page can optionally be provided
    if (!args['p'].isNull()) {
      m_page = strtol(args['p'].toString().data(), nullptr, 10);
    }
  }

  ~EvalCmd() {}

  void handleImpl(xdebug_xml_node& xml) override {
    // Do the eval. Throw on failure.
    Variant result;
    if (g_context->evalPHPDebugger((TypedValue*) &result,
                                   m_evalUnit, 0)) {
      throw XDebugServer::ERROR_EVALUATING_CODE;
    }

    // Construct the exporter
    XDebugExporter exporter;
    exporter.max_depth = m_server.m_maxDepth;
    exporter.max_children = m_server.m_maxChildren;
    exporter.max_data = m_server.m_maxData;
    exporter.page = m_page;

    // Create the xml node
    xdebug_xml_node* node = xdebug_get_value_xml_node(nullptr, result,
                                                      XDebugVarType::Normal,
                                                      exporter);
    xdebug_xml_add_child(&xml, node);
  }

private:
  Unit* m_evalUnit;
  int m_page = 0;
};

////////////////////////////////////////////////////////////////////////////////
// xcmd_profiler_name_get -i #
// Returns the profiler filename if profiling has started

class ProfilerNameGetCmd : public XDebugCommand {
public:
  ProfilerNameGetCmd(XDebugServer& server, const String& cmd, const Array& args)
    : XDebugCommand(server, cmd, args) {}
  ~ProfilerNameGetCmd() {}

  void handleImpl(xdebug_xml_node& xml) override {
    Variant filename = HHVM_FN(xdebug_get_profiler_filename)();
    if (!filename.isString()) {
      throw XDebugServer::ERROR_PROFILING_NOT_STARTED;
    }
    xdebug_xml_add_text(&xml, xdstrdup(filename.toString().data()));
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

bool XDebugCommand::handle(xdebug_xml_node& response) {
  m_server.addCommand(response, *this);
  handleImpl(response);
  return shouldContinue();
}

XDebugCommand* XDebugCommand::fromString(XDebugServer& server,
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
