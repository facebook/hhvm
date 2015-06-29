/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/xdebug/status.h"

#include "hphp/util/assertions.h"

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

const char* XDebugExn::what() const noexcept {
  return xdebug_error_str(error);
}

const char* xdebug_error_str(XDebugError error) {
  switch (error) {
    case XDebugError::Ok:
      return "no error";
    case XDebugError::Parse:
      return "parse error in command";
    case XDebugError::DupArg:
      return "duplicate arguments in command";
    case XDebugError::InvalidArgs:
      return "invalid or missing options";
    case XDebugError::Unimplemented:
      return "unimplemented command";
    case XDebugError::CommandUnavailable:
      return "command is not available";
    case XDebugError::CantOpenFile:
      return "can not open file";
    case XDebugError::StreamRedirectFailed:
      return "stream redirect failed";
    case XDebugError::BreakpointNotSet:
      return "breakpoint could not be set";
    case XDebugError::BreakpointTypeNotSupported:
      return "breakpoint type is not supported";
    case XDebugError::BreakpointInvalid:
      return "invalid breakpoint line";
    case XDebugError::BreakpointNoCode:
      return "no code on breakpoint line";
    case XDebugError::BreakpointInvalidState:
      return "invalid breakpoint state";
    case XDebugError::NoSuchBreakpoint:
      return "no such breakpoint";
    case XDebugError::EvaluatingCode:
      return "error evaluating code";
    case XDebugError::InvalidExpression:
      return "invalid expression";
    case XDebugError::PropertyNonExistent:
      return "can not get property";
    case XDebugError::StackDepthInvalid:
      return "stack depth invalid";
    case XDebugError::ContextInvalid:
      return "context invalid";
    case XDebugError::ProfilingNotStarted:
      return "profiler not started";
    case XDebugError::EncodingNotSupported:
      return "encoding not supported";
    case XDebugError::Internal:
      return "an internal exception in the debugger";
    case XDebugError::Unknown:
      return "unknown error";
  }
  not_reached();
}

const char* xdebug_status_str(XDebugStatus status) {
  switch (status) {
    case XDebugStatus::Starting: return "starting";
    case XDebugStatus::Stopping: return "stopping";
    case XDebugStatus::Stopped:  return "stopped";
    case XDebugStatus::Running:  return "running";
    case XDebugStatus::Break:    return "break";
    case XDebugStatus::Detached: return "detached";
  }
  not_reached();
}

const char* xdebug_reason_str(XDebugReason reason) {
  switch (reason) {
    case XDebugReason::Ok:        return "ok";
    case XDebugReason::Error:     return "error";
    case XDebugReason::Aborted:   return "aborted";
    case XDebugReason::Exception: return "exception";
  }
  not_reached();
}

void throw_exn(XDebugError error) {
  throw XDebugExn(error);
}

////////////////////////////////////////////////////////////////////////////////
}
