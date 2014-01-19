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

#include "hphp/runtime/base/zip-file.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/runtime-error.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

ZipFile::ZipFile() : m_gzFile(nullptr) {
  m_innerFile = NEWOBJ(PlainFile)();
  m_isLocal = true;
  m_eof = false;
}

ZipFile::~ZipFile() {
  ZipFile::closeImpl();
  delete m_innerFile;
}

void ZipFile::sweep() {
  closeImpl();
  m_innerFile = nullptr; // it'll get swept elsewhere
  File::sweep();
}

bool ZipFile::open(const String& filename, const String& mode) {
  assert(m_gzFile == nullptr);

  if (strchr(mode.c_str(), '+')) {
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
      m_gzFile = nullptr;
    }
    m_closed = true;
    m_innerFile->close();
  }
  File::closeImpl();
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

int64_t ZipFile::readImpl(char *buffer, int64_t length) {
  assert(m_gzFile);
  int64_t nread = gzread(m_gzFile, buffer, length);
  if (nread == 0 || gzeof(m_gzFile)) {
    m_eof = true;
  } else {
    errno = 0;
    gzerror(m_gzFile, &errno);
    if (errno == 1) { // Z_STREAM_END = 1
      m_eof = true;
    }
  }
  return (nread < 0) ? 0 : nread;
}

int64_t ZipFile::writeImpl(const char *buffer, int64_t length) {
  assert(m_gzFile);
  return gzwrite(m_gzFile, buffer, length);
}

bool ZipFile::seek(int64_t offset, int whence /* = SEEK_SET */) {
  assert(m_gzFile);

  if (whence == SEEK_CUR) {
    off_t result = gzseek(m_gzFile, 0, SEEK_CUR);
    if (result != (off_t)-1) {
      offset += result - (m_writepos - m_readpos + m_position);
    }
    if (offset > 0 && offset < m_writepos - m_readpos) {
      m_readpos += offset;
      m_position += offset;
      return true;
    }
    offset += m_position;
    whence = SEEK_SET;
  }

  // invalidate the current buffer
  m_writepos = 0;
  m_readpos = 0;
  m_eof = false;
  flush();
  off_t result = gzseek(m_gzFile, offset, whence);
  m_position = result;
  return result != (off_t)-1;
}

int64_t ZipFile::tell() {
  assert(m_gzFile);
  return m_position;
}

bool ZipFile::eof() {
  assert(m_gzFile);
  int64_t avail = m_writepos - m_readpos;
  return avail > 0 ? false : m_eof;
}

bool ZipFile::rewind() {
  assert(m_gzFile);
  seek(0);
  m_writepos = 0;
  m_readpos = 0;
  m_position = 0;
  m_eof = false;
  gzrewind(m_gzFile);
  return true;
}

bool ZipFile::flush() {
  assert(m_gzFile);
  return gzflush(m_gzFile, Z_SYNC_FLUSH);
}

///////////////////////////////////////////////////////////////////////////////
}
