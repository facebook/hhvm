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

#include "hphp/runtime/base/pipe.h"

#include "hphp/runtime/base/type-string.h"

#ifdef _MSC_VER
#include "hphp/util/process.h"
#else
#include "hphp/util/light-process.h"
#endif

namespace HPHP {

IMPLEMENT_RESOURCE_ALLOCATION(Pipe)
///////////////////////////////////////////////////////////////////////////////

Pipe::Pipe() {
}

Pipe::~Pipe() {
  closeImpl();
}

bool Pipe::open(const String& filename, const String& mode) {
  assert(m_stream == nullptr);
  assert(getFd() == -1);

#ifdef _MSC_VER
  auto old_cwd = Process::GetCurrentDirectory();
  chdir(g_context->getCwd().data());
  FILE* f = _popen(filename.data(), mode.data());
  chdir(old_cwd.c_str());
#else
  FILE *f = LightProcess::popen(filename.data(), mode.data(),
                                g_context->getCwd().data());
#endif
  if (!f) {
    return false;
  }
  m_stream = f;
  setFd(fileno(f));
  return true;
}

bool Pipe::close() {
  invokeFiltersOnClose();
  return closeImpl();
}

bool Pipe::closeImpl() {
  bool ret = true;
  s_pcloseRet = 0;
  if (valid() && !isClosed()) {
    assert(m_stream);
#ifdef _MSC_VER
    int pcloseRet = _pclose(m_stream);
#else
    int pcloseRet = LightProcess::pclose(m_stream);
    if (WIFEXITED(pcloseRet)) pcloseRet = WEXITSTATUS(pcloseRet);
#endif
    s_pcloseRet = pcloseRet;
    ret = (pcloseRet == 0);
    setIsClosed(true);
    m_stream = nullptr;
  }
  File::closeImpl();
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
