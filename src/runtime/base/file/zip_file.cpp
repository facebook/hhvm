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

#include <runtime/base/file/zip_file.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/runtime_error.h>

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP(ZipFile);
///////////////////////////////////////////////////////////////////////////////

StaticString ZipFile::s_class_name("ZipFile");

///////////////////////////////////////////////////////////////////////////////

ZipFile::ZipFile() : m_gzFile(NULL) {
  m_innerFile = NEWOBJ(PlainFile)();
  m_innerFile->unregister(); // so Sweepable won't touch my child
}

ZipFile::~ZipFile() {
  closeImpl();
  m_innerFile->~PlainFile();
  DELETEOBJ(HPHP, PlainFile, m_innerFile);
}

void ZipFile::sweep() {
  closeImpl();
  File::closeImpl();
}

bool ZipFile::open(CStrRef filename, CStrRef mode) {
  ASSERT(m_gzFile == NULL);

  if (strchr(mode, '+')) {
    raise_warning("cannot open a zlib stream for reading and writing "
                    "at the same time!");
    return false;
  }

  return m_innerFile->open(filename, mode) &&
    (m_gzFile = gzdopen(dup(m_innerFile->fd()), mode.data()));
}

bool ZipFile::close() {
  return closeImpl();
}

bool ZipFile::closeImpl() {
  bool ret = true;
  s_file_data->m_pcloseRet = 0;
  if (!m_closed) {
    if (m_gzFile) {
      s_file_data->m_pcloseRet = gzclose(m_gzFile);
      ret = (s_file_data->m_pcloseRet == 0);
      m_gzFile = NULL;
    }
    m_closed = true;
    m_innerFile->close();
  }
  File::closeImpl();
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

int64 ZipFile::readImpl(char *buffer, int64 length) {
  ASSERT(m_gzFile);
  return gzread(m_gzFile, buffer, length);
}

int64 ZipFile::writeImpl(const char *buffer, int64 length) {
  ASSERT(m_gzFile);
  return gzwrite(m_gzFile, buffer, length);
}

bool ZipFile::seek(int64 offset, int whence /* = SEEK_SET */) {
  ASSERT(m_gzFile);
  int64 newoffset = gzseek(m_gzFile, offset, whence);
  return (newoffset < 0) ? -1 : 0;
}

int64 ZipFile::tell() {
  ASSERT(m_gzFile);
  return gztell(m_gzFile);
}

bool ZipFile::eof() {
  ASSERT(m_gzFile);
  return gzeof(m_gzFile);
}

bool ZipFile::rewind() {
  ASSERT(m_gzFile);
  gzrewind(m_gzFile);
  return true;
}

bool ZipFile::flush() {
  ASSERT(m_gzFile);
  return gzflush(m_gzFile, Z_SYNC_FLUSH);
}

///////////////////////////////////////////////////////////////////////////////
}
