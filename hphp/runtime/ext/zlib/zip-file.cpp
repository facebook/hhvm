/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/zlib/zip-file.h"
#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/temp-file.h"
#include "hphp/runtime/base/runtime-error.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

ZipFile::ZipFile() : m_gzFile(nullptr) {
  setIsLocal(true);
  setEof(false);
}

ZipFile::~ZipFile() {
  ZipFile::closeImpl();
}

void ZipFile::sweep() {
  closeImpl();
  File::sweep();
}

bool ZipFile::open(const String& filename, const String& mode) {
  assert(m_gzFile == nullptr);

  if (strchr(mode.c_str(), '+')) {
    raise_warning("cannot open a zlib stream for reading and writing "
                    "at the same time!");
    return false;
  }

  m_innerFile = File::Open(filename, mode);
  if (isa<MemFile>(m_innerFile)) {
    // We need an FD for the correct zlib APIs; MemFiles don't have an FD
    if (strchr(mode.c_str(), 'w')) {
      raise_warning("Cannot write to this stream type");
      return false;
    }
    auto file = req::make<TempFile>();
    while (!m_innerFile->eof()) {
      file->write(m_innerFile->read(File::CHUNK_SIZE));
    }
    file->rewind();
    m_tempFile = file;
    return (m_gzFile = gzdopen(dup(file->fd()), mode.data()));
  }
  if(m_innerFile) {
    m_tempFile.reset();
    return (m_gzFile = gzdopen(dup(m_innerFile->fd()), mode.data()));
  }
  return false;
}

bool ZipFile::close() {
  invokeFiltersOnClose();
  return closeImpl();
}

bool ZipFile::closeImpl() {
  bool ret = true;
  s_pcloseRet = 0;
  if (!isClosed()) {
    if (m_gzFile) {
      s_pcloseRet = gzclose(m_gzFile);
      ret = (s_pcloseRet == 0);
      m_gzFile = nullptr;
    }
    setIsClosed(true);
    if (m_innerFile) {
      m_innerFile->close();
    }
    if (m_tempFile) {
      m_tempFile->close();
      m_tempFile.reset();
    }
  }
  File::closeImpl();
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

int64_t ZipFile::readImpl(char *buffer, int64_t length) {
  assert(m_gzFile);
  int64_t nread = gzread(m_gzFile, buffer, length);
  if (nread == 0 || gzeof(m_gzFile)) {
    setEof(true);
  } else {
    errno = 0;
    gzerror(m_gzFile, &errno);
    if (errno == 1) { // Z_STREAM_END = 1
      setEof(true);
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
      offset += result - (bufferedLen() + getPosition());
    }
    if (offset > 0 && offset < bufferedLen()) {
      setReadPosition(getReadPosition() + offset);
      setPosition(getPosition() + offset);
      return true;
    }
    offset += getPosition();
    whence = SEEK_SET;
  }

  // invalidate the current buffer
  setWritePosition(0);
  setReadPosition(0);
  setEof(false);
  gzclearerr(m_gzFile);
  flush();
  off_t result = gzseek(m_gzFile, offset, whence);
  setPosition(result);
  return result != (off_t)-1;
}

int64_t ZipFile::tell() {
  assert(m_gzFile);
  return getPosition();
}

bool ZipFile::eof() {
  assert(m_gzFile);
  int64_t avail = bufferedLen();
  return avail > 0 ? false : getEof();
}

bool ZipFile::rewind() {
  assert(m_gzFile);
  seek(0);
  setWritePosition(0);
  setReadPosition(0);
  setPosition(0);
  setEof(false);
  gzrewind(m_gzFile);
  return true;
}

bool ZipFile::flush() {
  assert(m_gzFile);
  return gzflush(m_gzFile, Z_SYNC_FLUSH);
}

///////////////////////////////////////////////////////////////////////////////
}
