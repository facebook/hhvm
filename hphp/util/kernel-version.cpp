/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/assertions.h"
#include "hphp/util/portability.h"

#include <stdio.h>
#include <iostream>

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/utsname.h>
#endif

namespace HPHP {

void KernelVersion::parse(const char* s) {
  char dashdot[2];
  // Assume no part of the uname string will be more than 128 chars
  char release[128];
  release[0] = 0;
  char build[128];
  build[0] = 0;
  sscanf(s,
         "%d.%d%1[.-]%127[A-Za-z0-9]-%127[A-Za-z0-9]_fbk%d_",
         &m_major, &m_minor, dashdot, release, build, &m_fbk);
  m_release_str = release[0] == 0 ? "" : (const char*) release;
  m_build_str = build[0] == 0 ? "" : (const char*) build;
  // Populate the int-based release and build, if we have all digits in them
  if (isNumber(m_release_str)) {
    m_release = atoi(m_release_str.c_str());
  }
  if (isNumber(m_build_str)) {
    m_build = atoi(m_build_str.c_str());
  }
}

KernelVersion::KernelVersion() {
#ifdef _MSC_VER
  OSVERSIONINFO verInf;
  verInf.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&verInf);
  m_major = verInf.dwMajorVersion;
  m_minor = verInf.dwMinorVersion;
  m_build = verInf.dwBuildNumber;
  m_release = verInf.dwPlatformId;
#else
  struct utsname uts;
  DEBUG_ONLY int err = uname(&uts);
  assert(err == 0);
  parse(uts.release);
#endif
}

KernelVersion::KernelVersion(const char* s) {
  m_major = m_minor = m_fbk = m_release = m_build = -1;
  m_release_str = m_build_str = "";
  parse(s);
}

bool KernelVersion::isNumber(const std::string s) {
  return !s.empty() && s.find_first_not_of("0123456789") == std::string::npos;
}

}
