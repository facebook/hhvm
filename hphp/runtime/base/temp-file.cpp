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

#include "hphp/runtime/base/temp-file.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/runtime-error.h"

#include "hphp/util/logger.h"

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
  setFd(fd);
  m_stream = fdopen(fd, "r+");
  setName(path);
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

bool TempFile::open(const String& /*filename*/, const String& /*mode*/) {
  raise_fatal_error((std::string("cannot open a temp file ") +
                             getName()).c_str());
}

bool TempFile::close() {
  invokeFiltersOnClose();
  return closeImpl();
}

bool TempFile::closeImpl() {
  bool ret = true;
  s_pcloseRet = 0;
  if (!isClosed()) {
    assert(valid());
    s_pcloseRet = ::fclose(m_stream);
    ret = (s_pcloseRet == 0);
    setIsClosed(true);
    m_stream = nullptr;
    setFd(-1);
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

bool TempFile::seek(int64_t offset, int whence /* = SEEK_SET */) {
  assert(valid());

  if (whence == SEEK_CUR) {
    off_t result = lseek(getFd(), 0, SEEK_CUR);
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
  } else if (whence == SEEK_END) {
    if (getLength() == -1) {
      Logger::Verbose("%s/%d: error finding end of file", __FUNCTION__,
                       __LINE__);
      return false;
    }
    offset += getLength();
    whence = SEEK_SET;
  }

  if (offset > getLength() || getLength() == -1) return false;

  // invalidate the current buffer
  setWritePosition(0);
  setReadPosition(0);
  // clear the eof flag
  setEof(false);
  flush();
  // lseek instead of seek to be consistent with read
  off_t result = lseek(getFd(), offset, whence);
  setPosition(result);
  return result != (off_t)-1;
}

int64_t TempFile::tell() {
  assert(valid());
  if (getLength() < 0) return -1;
  return getPosition();
}

bool TempFile::truncate(int64_t size) {
  assert(valid());
  seek(size, SEEK_SET);
  return ftruncate(getFd(), size) == 0;
}

int64_t TempFile::getLength() {
  struct stat sb;
  if (StatCache::lstat(File::TranslatePathWithFileCache(m_rawName).c_str(), &sb)) {
    Logger::Verbose("%s/%d: %s", __FUNCTION__, __LINE__,
                    folly::errnoStr(errno).c_str());
    // use fstat directly
    if (fstat(getFd(), &sb) != 0) return -1;
    return sb.st_size;
  }
  return sb.st_size;
}

///////////////////////////////////////////////////////////////////////////////
}
