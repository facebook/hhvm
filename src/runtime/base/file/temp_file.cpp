/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/file/temp_file.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/runtime_error.h>

using namespace std;

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION(TempFile)
///////////////////////////////////////////////////////////////////////////////

StaticString TempFile::s_class_name("TempFile");

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

TempFile::TempFile(bool autoDelete /* = true */) : m_autoDelete(autoDelete) {
  char path[PATH_MAX];

  // open a temporary file
  snprintf(path, sizeof(path), "/tmp/XXXXXX");
  int fd = mkstemp(path);
  if (fd == -1) {
    raise_warning("Unable to open temporary file");
    return;
  }
  m_fd = fd;
  m_name = string(path);
}

TempFile::~TempFile() {
  closeImpl();
}

bool TempFile::open(CStrRef filename, CStrRef mode) {
  throw FatalErrorException((string("cannot open a temp file ") +
                             m_name).c_str());
}

bool TempFile::close() {
  return closeImpl();
}

bool TempFile::closeImpl() {
  bool ret = true;
  s_file_data->m_pcloseRet = 0;
  if (!m_closed) {
    ASSERT(valid());
    s_file_data->m_pcloseRet = ::close(m_fd);
    ret = (s_file_data->m_pcloseRet == 0);
    m_closed = true;
    m_stream = NULL;
    m_fd = -1;
  }
  if (!m_name.empty()) {
    if (m_autoDelete) {
      unlink(m_name.c_str());
    }
    m_name.clear();
  }
  File::closeImpl();
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
