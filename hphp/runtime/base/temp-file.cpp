/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/temp-file.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/runtime-error.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

TempFile::TempFile(bool autoDelete /* = true */,
                   const String& wrapper_type,
                   const String& stream_type)
  : PlainFile(nullptr, false, wrapper_type, stream_type),
    m_autoDelete(autoDelete) {
  char path[PATH_MAX];

  // open a temporary file
  snprintf(path, sizeof(path), "/tmp/XXXXXX");
  int fd = mkstemp(path);
  if (fd == -1) {
    raise_warning("Unable to open temporary file");
    return;
  }
  m_fd = fd;
  m_stream = fdopen(fd, "r+");
  m_name = std::string(path);
  m_rawName = std::string(path);
}

TempFile::~TempFile() {
  closeImpl();
}

void TempFile::sweep() {
  closeImpl();
  using std::string;
  m_rawName.~string();
  PlainFile::sweep();
}

bool TempFile::open(const String& filename, const String& mode) {
  throw FatalErrorException((std::string("cannot open a temp file ") +
                             m_name).c_str());
}

bool TempFile::close() {
  invokeFiltersOnClose();
  return closeImpl();
}

bool TempFile::closeImpl() {
  bool ret = true;
  s_file_data->m_pcloseRet = 0;
  if (!m_closed) {
    assert(valid());
    s_file_data->m_pcloseRet = ::close(m_fd);
    ret = (s_file_data->m_pcloseRet == 0);
    m_closed = true;
    m_stream = nullptr;
    m_fd = -1;
  }
  if (!m_rawName.empty()) {
    if (m_autoDelete) {
      unlink(m_rawName.c_str());
    }
    m_rawName.clear();
  }
  File::closeImpl();
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
