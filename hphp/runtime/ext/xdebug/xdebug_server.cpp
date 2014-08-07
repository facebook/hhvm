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

#include "hphp/runtime/ext/xdebug/xdebug_server.h"

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

// Globals
const static StaticString
  s_SERVER("_SERVER"),
  s_GET("_GET"),
  s_COOKIE("_COOKIE");

///////////////////////////////////////////////////////////////////////////////
// Construction/Destruction

XDebugServer::XDebugServer(Mode mode) {}
XDebugServer::~XDebugServer() {}

///////////////////////////////////////////////////////////////////////////////
// Statics

// Server session properties
static const StaticString
  s_SESSION_START("XDEBUG_SESSION_START"),
  s_SESSION_STOP("XDEBUG_SESSION_STOP"),
  s_SESSION("XDEBUG_SESSION");

void XDebugServer::onRequestInit() {
  if (!XDEBUG_GLOBAL(RemoteEnable)) {
    return;
  }

  // TODO(#4489053) Enable this when debugger internals have been refactored
  // Need to turn on debugging regardless of the remote mode in order to
  // capture exceptions/errors
  // ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  // ti->m_reqInjectionData.setDebugger(true);

  // Grab $_GET, $_COOKIE, and the transport
  const ArrayData* globals = get_global_variables()->asArrayData();
  Array get = globals->get(s_GET).toArray();
  Array cookie = globals->get(s_COOKIE).toArray();
  Transport* transport = g_context->getTransport();

  // Need to check $_GET[XDEBUG_SESSION_STOP]. If set, delete the session
  // cookie
  const Variant sess_stop_var = get[s_SESSION_STOP];
  if (!sess_stop_var.isNull()) {
    cookie.set(s_SESSION, init_null());
    if (transport != nullptr) {
      transport->setCookie(s_SESSION, empty_string());
    }
  }

  // Need to check $_GET[XDEBUG_SESSION_START]. If set, store the session
  // cookie with $_GET[XDEBUG_SESSION_START] as the value
  const Variant sess_start_var = get[s_SESSION_START];
  if (sess_start_var.isString()) {
    String sess_start = sess_start_var.toString();
    cookie.set(s_SESSION,  sess_start);
    if (transport != nullptr) {
      transport->setCookie(s_SESSION,
                           sess_start,
                           XDEBUG_GLOBAL(RemoteCookieExpireTime));
    }
  }
}

bool XDebugServer::isNeeded() {
  if (!XDEBUG_GLOBAL(RemoteEnable) ||
      XDEBUG_GLOBAL(RemoteMode) == "jit") {
    return false;
  } else if (XDEBUG_GLOBAL(RemoteAutostart)) {
    return true;
  } else {
    // Check $_COOKIE[XDEBUG_SESSION]
    const ArrayData* globals = get_global_variables()->asArrayData();
    Array cookie = globals->get(s_COOKIE).toArray();
    return !cookie[s_SESSION].isNull();
  }
}

///////////////////////////////////////////////////////////////////////////////
}
