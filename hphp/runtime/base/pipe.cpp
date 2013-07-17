/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/pipe.h"
#include "hphp/runtime/base/complex_types.h"
#include "hphp/util/light_process.h"

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION(Pipe)
///////////////////////////////////////////////////////////////////////////////

StaticString Pipe::s_class_name("Pipe");

Pipe::Pipe() {
}

Pipe::~Pipe() {
  closeImpl();
}

bool Pipe::open(CStrRef filename, CStrRef mode) {
  assert(m_stream == nullptr);
  assert(m_fd == -1);

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
  s_file_data->m_pcloseRet = 0;
  if (!m_closed) {
    assert(m_stream);
    int pcloseRet = LightProcess::pclose(m_stream);
    if (WIFEXITED(pcloseRet)) pcloseRet = WEXITSTATUS(pcloseRet);
    s_file_data->m_pcloseRet = pcloseRet;
    ret = (pcloseRet == 0);
    m_closed = true;
    m_stream = nullptr;
  }
  File::closeImpl();
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
