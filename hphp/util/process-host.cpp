/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/util/process-host.h"

#include <sys/wait.h>
#include <unistd.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

std::string Process::GetHostName() {
  char hostbuf[128];
  hostbuf[0] = '\0'; // for cleaner valgrind output when gethostname() fails
  gethostname(hostbuf, sizeof(hostbuf));
  hostbuf[sizeof(hostbuf) - 1] = '\0';
  return hostbuf;
}

///////////////////////////////////////////////////////////////////////////////
}
