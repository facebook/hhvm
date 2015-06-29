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

#ifndef incl_HPHP_XDEBUG_STATUS_H_
#define incl_HPHP_XDEBUG_STATUS_H_

#include <exception>

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

/* Current state of the xdebug server. */
enum class XDebugStatus {
  Starting,
  Stopping,
  Stopped,
  Running,
  Break,
  Detached,
};

/* Reason for the current state. */
enum class XDebugReason {
  Ok,
  Error,
  Aborted,
  Exception,
};

/* See http://xdebug.org/docs-dbgp.php#error-codes */
enum class XDebugError {
  Ok = 0,
  Parse = 1,
  DupArg = 2,
  InvalidArgs = 3,
  Unimplemented = 4,
  CommandUnavailable = 5,

  CantOpenFile = 100,
  StreamRedirectFailed = 101,

  BreakpointNotSet = 200,
  BreakpointTypeNotSupported = 201,
  BreakpointInvalid = 202,
  BreakpointNoCode = 203,
  BreakpointInvalidState = 204,
  NoSuchBreakpoint = 205,
  EvaluatingCode = 206,
  InvalidExpression = 207,

  PropertyNonExistent = 300,
  StackDepthInvalid = 301,
  ContextInvalid = 302,

  ProfilingNotStarted = 800,

  EncodingNotSupported = 900,
  Internal = 998,
  Unknown = 999
};

/* An exception wrapper over XDebugError. */
struct XDebugExn : std::exception {
  explicit XDebugExn(XDebugError error) : error(error) {}

  const char* what() const noexcept override;

  XDebugError error;
};

////////////////////////////////////////////////////////////////////////////////

/* Stringify XDebug status codes. */
const char* xdebug_status_str(XDebugStatus);
const char* xdebug_reason_str(XDebugReason);
const char* xdebug_error_str(XDebugError);

/* Helper for throwing XDebug errors. */
void throw_exn(XDebugError);

////////////////////////////////////////////////////////////////////////////////
}

#endif
