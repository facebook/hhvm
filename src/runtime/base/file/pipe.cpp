/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/base/file/pipe.h>
#include <runtime/base/complex_types.h>
#include <util/light_process.h>

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION(Pipe)
///////////////////////////////////////////////////////////////////////////////

Pipe::Pipe() {
}

Pipe::~Pipe() {
  closeImpl();
}

bool Pipe::open(CStrRef filename, CStrRef mode) {
  ASSERT(m_stream == NULL);
  ASSERT(m_fd == -1);

  FILE *f = LightProcess::popen(filename.data(), mode.data());
  if (!f) {
    return false;
  }
  m_stream = f;
  m_fd = fileno(f);
  return true;
}

bool Pipe::close() {
  return closeImpl();
}

bool Pipe::closeImpl() {
  bool ret = true;
  if (!m_closed) {
    ASSERT(m_stream);
    ret = (LightProcess::pclose(m_stream) == 0);
    m_closed = true;
    m_stream = NULL;
  }
  File::closeImpl();
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
