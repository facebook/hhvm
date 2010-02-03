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

#include <cpp/base/file/zip_file.h>
#include <cpp/base/type_string.h>
#include <util/logger.h>

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP(ZipFile);
///////////////////////////////////////////////////////////////////////////////

ZipFile::ZipFile() : m_gzFile(NULL) {
  m_innerFile = NEW(PlainFile)();
  m_innerFile->unregister(); // so Sweepable won't touch my child
}

ZipFile::~ZipFile() {
  closeImpl();
  DELETE(PlainFile)(m_innerFile);
}

void ZipFile::sweep() {
  closeImpl();
  File::closeImpl();
}

bool ZipFile::open(CStrRef filename, CStrRef mode) {
  ASSERT(m_gzFile == NULL);

  if (strchr(mode, '+')) {
    Logger::Warning("cannot open a zlib stream for reading and writing "
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
  if (!m_closed) {
    if (m_gzFile) {
      ret = (gzclose(m_gzFile) == 0);
      m_gzFile = NULL;
    }
    m_closed = true;
    m_innerFile->close();
  }
  File::closeImpl();
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

int ZipFile::readImpl(char *buffer, int length) {
  ASSERT(m_gzFile);
  return gzread(m_gzFile, buffer, length);
}

int ZipFile::writeImpl(const char *buffer, int length) {
  ASSERT(m_gzFile);
  return gzwrite(m_gzFile, buffer, length);
}

bool ZipFile::seek(int offset, int whence /* = SEEK_SET */) {
  ASSERT(m_gzFile);
  int newoffset = gzseek(m_gzFile, offset, whence);
  return (newoffset < 0) ? -1 : 0;
}

int ZipFile::tell() {
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
