/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/kernel-version.h"
#include "hphp/util/util.h"
#include "hphp/util/assertions.h"

#include <unistd.h>
#include <sys/utsname.h>
#include <stdio.h>

namespace HPHP {

void KernelVersion::parse(const char* s) {
  DEBUG_ONLY int numFields = sscanf(s,
                                    "%d.%d.%d-%d_fbk%d_",
                                    &m_major, &m_dot, &m_dotdot, &m_dash,
                                    &m_fbk);
  assert(numFields >= 3);
}

KernelVersion::KernelVersion() {
  struct utsname uts;
  DEBUG_ONLY int err = uname(&uts);
  assert(err == 0);
  m_major = m_dot = m_dotdot = m_dash = m_fbk = -1;
  parse(uts.release);
}

KernelVersion::KernelVersion(const char* s) {
  parse(s);
}

}
