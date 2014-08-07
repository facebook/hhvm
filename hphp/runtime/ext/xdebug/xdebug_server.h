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

#ifndef incl_HPHP_XDEBUG_SERVER_H_
#define incl_HPHP_XDEBUG_SERVER_H_

#include "hphp/runtime/ext/xdebug/ext_xdebug.h"
#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_xml.h"

#define DBGP_VERSION "1.0"

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

class XDebugServer {
////////////////////////////////////////////////////////////////////////////////
// Construction/Destruction

public:
  enum class Mode {
    REQ, // Server created during request init
    JIT // Server created on demand
  };

  // An XDebugServer is only valid if the constructor succeeds. An exception is
  // thrown otherwise. The constructor is responsible for establishing a valid
  // dbgp connection with the client
  explicit XDebugServer(Mode mode);
  ~XDebugServer();

////////////////////////////////////////////////////////////////////////////////
// Statics

public:
  // Request specific initialization
  static void onRequestInit();

  // Returns true if the xdebug server is needed by this thread. If remote_mode
  // is "jit" then this always returns false as whether or not the server is
  // needed is decided at runtime.
  static bool isNeeded();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_XDEBUG_SERVER_H_
