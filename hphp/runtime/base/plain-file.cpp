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
#include "hphp/runtime/base/plain-file.h"

#include <sys/types.h>
#include <sys/stat.h>

#include "hphp/util/rds-local.h"

#include <folly/portability/Fcntl.h>
#include <folly/portability/Stdio.h>
#include <folly/portability/Unistd.h>

namespace HPHP {

const StaticString s_plainfile("plainfile");
const StaticString s_stdio("STDIO");
const StaticString s_stdin("STDIN");
const StaticString s_stdout("STDOUT");
const StaticString s_stderr("STDERR");

struct StdFiles {
  FILE* stdin{nullptr};
  FILE* stdout{nullptr};
  FILE* stderr{nullptr};
};

namespace {
RDS_LOCAL(StdFiles, rl_stdfiles);
}

void setThreadLocalIO(FILE* in, FILE* out, FILE* err) {
  // Before setting new thread local IO structures the previous ones must be
  // cleared to ensure that they are closed appropriately.
  always_assert(!rl_stdfiles->stdin &&
                !rl_stdfiles->stdout &&
                !rl_stdfiles->stderr);

  rl_stdfiles->stdin = in;
  rl_stdfiles->stdout = out;
  rl_stdfiles->stderr = err;
}

void clearThreadLocalIO() {
  rl_stdfiles->stdin = rl_stdfiles->stdout = rl_stdfiles->stderr = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

PlainFile::PlainFile(FILE *stream, bool nonblocking,
                     const String& wrapper_type, const String& stream_type)
  : File(nonblocking,
         wrapper_type.isNull() ? s_plainfile : wrapper_type,
         stream_type.isNull() ? s_stdio : stream_type),
    m_stream(stream), m_buffer(nullptr) {
  if (stream) {
    setFd(fileno(stream));
    m_buffer = (char *)malloc(BUFSIZ);
    if (m_buffer)
      setbuffer(stream, m_buffer, BUFSIZ);
  }
  setIsLocal(true);
}

PlainFile::PlainFile(int fd, bool nonblocking,
                     const String& wrapper_type, const String& stream_type)
  : File(nonblocking,
         wrapper_type.isNull() ? s_plainfile : wrapper_type,
         stream_type.isNull() ? s_stdio : stream_type),
    m_stream(nullptr), m_buffer(nullptr) {
  setFd(fd);
}

PlainFile::~PlainFile() {
  PlainFile::close();
}

void PlainFile::sweep() {
  PlainFile::close();
  File::sweep();
}

bool PlainFile::open(const String& filename, const String& mode) {
  int fd;
  FILE *f;
  assertx(m_stream == nullptr);
  assertx(getFd() == -1);

  // For these defined in php fopen but C stream have different modes
  switch (mode[0]) {
    case 'x':
      if (mode.find('+') == -1) {
        fd = ::open(filename.data(), O_WRONLY|O_CREAT|O_EXCL, 0666);
        if (fd < 0) return false;
        f = fdopen(fd, "w");
      } else {
        fd = ::open(filename.data(), O_RDWR|O_CREAT|O_EXCL, 0666);
        if (fd < 0) return false;
        f = fdopen(fd, "w+");
      }
      break;
    case 'c':
      if (mode.find('+') == -1) {
        fd = ::open(filename.data(), O_WRONLY|O_CREAT, 0666);
        if (fd < 0) return false;
        f = fdopen(fd, "w");
      } else {
        fd = ::open(filename.data(), O_RDWR|O_CREAT, 0666);
        if (fd < 0) return false;
        f = fdopen(fd, "w+");
      }
      break;
    default:
      f = fopen(filename.data(), mode.data());
  }
  if (!f) {
    return false;
  }
  m_stream = f;
  setFd(fileno(f));
  m_buffer = (char *)malloc(BUFSIZ);
  setName(filename.toCppString());
  if (m_buffer)
    setbuffer(f, m_buffer, BUFSIZ);
  return true;
}

bool PlainFile::close(int*) {
  bool ret = true;
  if (!isClosed()) {
    if (m_stream) {
      ret = (fclose(m_stream) == 0);
      m_stream = nullptr;
    } else if (getFd() >= 0) {
      ret = (::close(getFd()) == 0);
    }
    if (m_buffer) {
      free(m_buffer);
      m_buffer = nullptr;
    }
    setIsClosed(true);
    setFd(-1);
  }
  File::close();
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// virtual functions

int64_t PlainFile::readImpl(char *buffer, int64_t length) {
  assertx(valid());
  assertx(length > 0);
  // use read instead of fread to handle EOL in stdin
  size_t ret = ::read(getFd(), buffer, length);
  if (ret == 0
   || (ret == (size_t)-1
    && errno != EWOULDBLOCK && errno != EINTR && errno != EBADF)) {
    setEof(true);
  }
  return ret == (size_t)-1 ? 0 : ret;
}

int PlainFile::getc() {
  assertx(valid());
  return File::getc();
}

// This definition is needed to avoid triggering a gcc compiler error about
// an overloaded virtual when only overriding the one parameter version from
// File.
String PlainFile::read() {
  return File::read();
}

String PlainFile::read(int64_t length) {
  if (length) setEof(false);
  return File::read(length);
}

int64_t PlainFile::writeImpl(const char *buffer, int64_t length) {
  assertx(valid());
  assertx(length > 0);

  // use write instead of fwrite to be consistent with read
  // o.w., read-and-write files would not work
  int64_t written = ::write(getFd(), buffer, length);
  return written < 0 ? 0 : written;
}

bool PlainFile::seek(int64_t offset, int whence /* = SEEK_SET */) {
  assertx(valid());

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
  }

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

int64_t PlainFile::tell() {
  assertx(valid());
  return getPosition();
}

bool PlainFile::eof() {
  assertx(valid());
  int64_t avail = bufferedLen();
  if (avail > 0) {
    return false;
  }
  return getEof();
}

bool PlainFile::rewind() {
  assertx(valid());
  seek(0);
  setWritePosition(0);
  setReadPosition(0);
  setPosition(0);
  return true;
}

bool PlainFile::stat(struct stat *sb) {
  assertx(valid());
  return ::fstat(getFd(), sb) == 0;
}

bool PlainFile::flush() {
  if (m_stream) {
    return fflush(m_stream) == 0;
  }
  assertx(valid());
  // No need to flush a file descriptor.
  return true;
}

bool PlainFile::truncate(int64_t size) {
  assertx(valid());
  return ftruncate(getFd(), size) == 0;
}

///////////////////////////////////////////////////////////////////////////////
// BuiltinFiles

const StaticString s_php("PHP");

BuiltinFile::BuiltinFile(FILE *stream) : PlainFile(stream, true, s_php) {}
BuiltinFile::BuiltinFile(int fd) : PlainFile(fd, true, s_php) {}

BuiltinFile::~BuiltinFile() {
  setIsClosed(true);
  m_stream = nullptr;
  setFd(-1);
}

bool BuiltinFile::close(int*) {
  int status = 0;
       if (m_stream == rl_stdfiles->stdin)  rl_stdfiles->stdin = nullptr;
  else if (m_stream == rl_stdfiles->stdout) rl_stdfiles->stdout = nullptr;
  else if (m_stream == rl_stdfiles->stderr) rl_stdfiles->stderr = nullptr;
  else                                      status = ::fclose(m_stream);
  setIsClosed(true);
  m_stream = nullptr;
  setFd(-1);
  File::close();
  return status == 0;
}

void BuiltinFile::sweep() {
  // This object was just a wrapper around a FILE* or fd owned by someone else,
  // so don't close it except in explicit calls to close(). Beware this doesn't
  // call PlainFile::sweep().
  m_stream = nullptr;
  setFd(-1);
  setIsClosed(true);
  File::sweep();
}

IMPLEMENT_REQUEST_LOCAL(BuiltinFiles, g_builtin_files);

void BuiltinFiles::requestInit() {
  getSTDIN();
  getSTDOUT();
  getSTDERR();
}

void BuiltinFiles::requestShutdown() {
  m_stdin.releaseForSweep();
  m_stdout.releaseForSweep();
  m_stderr.releaseForSweep();
}

static const Variant& getHelper(Variant& global_fd, FILE* rds_fd, FILE* fd,
                         int fd_num) {
  if (global_fd.isNull()) {
    auto f = req::make<BuiltinFile>(rds_fd ? rds_fd : fd);
    global_fd = f;
    f->setId(fd_num);
    assertx(f->getId() == fd_num);
  }
  return global_fd;
}

Variant BuiltinFiles::getSTDIN(const StringData* name) {
  assertx(s_stdin.same(name));
  return getSTDIN();
}

Variant BuiltinFiles::getSTDOUT(const StringData* name) {
  assertx(s_stdout.same(name));
  return getSTDOUT();
}

Variant BuiltinFiles::getSTDERR(const StringData* name) {
  assertx(s_stderr.same(name));
  return getSTDERR();
}

const Variant& BuiltinFiles::getSTDIN() {
  return getHelper(g_builtin_files->m_stdin, rl_stdfiles->stdin, stdin, 1);
}

const Variant& BuiltinFiles::getSTDOUT() {
  return getHelper(g_builtin_files->m_stdout, rl_stdfiles->stdout, stdout, 2);
}

const Variant& BuiltinFiles::getSTDERR() {
  return getHelper(g_builtin_files->m_stderr, rl_stdfiles->stderr, stderr, 3);
}

///////////////////////////////////////////////////////////////////////////////
}
