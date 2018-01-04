/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017-present Facebook, Inc. (http://www.facebook.com)  |
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

#include <unistd.h>
#include <fcntl.h>

#include "hphp/runtime/ext/vsdebug/fdtransport.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"

namespace HPHP {
namespace VSDEBUG {

FdTransport::FdTransport(Debugger* debugger) :
  DebugTransport(debugger) {

  errno = 0;
  if (fcntl(TransportFd, F_GETFL) != -1 && errno == 0) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelInfo,
      "Opened transport via file descriptor %u.",
      TransportFd
    );
    setTransportFd(TransportFd);
  } else {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Opening transport via file descriptor %u failed: %d",
      TransportFd,
      errno
    );
    throw Exception("Failed to open file descriptor transport.");
  }
}

bool FdTransport::clientConnected() const {
  return getTransportFd() >= 0;
}

}
}
