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
#include "hphp/runtime/base/plain-file.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/request-local.h"

namespace HPHP {

const StaticString s_plainfile("plainfile");
const StaticString s_stdio("STDIO");

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

PlainFile::PlainFile(FILE *stream, bool nonblocking,
                     const String& wrapper_type, const String& stream_type)
  : File(nonblocking,
         wrapper_type.isNull() ? s_plainfile : wrapper_type,
         stream_type.isNull() ? s_stdio : stream_type),
    m_stream(stream), m_buffer(nullptr) {
  if (stream) {
    m_fd = fileno(stream);
    m_buffer = (char *)malloc(BUFSIZ);
    if (m_buffer)
      setbuffer(stream, m_buffer, BUFSIZ);
  }
  m_isLocal = true;
}

PlainFile::PlainFile(int fd, bool nonblocking,
                     const String& wrapper_type, const String& stream_type)
  : File(nonblocking,
         wrapper_type.isNull() ? s_plainfile : wrapper_type,
         stream_type.isNull() ? s_stdio : stream_type),
    m_stream(nullptr), m_buffer(nullptr) {
  m_fd = fd;
}

PlainFile::~PlainFile() {
  closeImpl();
}

void PlainFile::sweep() {
  closeImpl();
  File::sweep();
}

bool PlainFile::open(const String& filename, const String& mode) {
  int fd;
  FILE *f;
  assert(m_stream == nullptr);
  assert(m_fd == -1);

  // For these definded in php fopen but C stream have different modes
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
  m_fd = fileno(f);
  m_buffer = (char *)malloc(BUFSIZ);
  m_name = static_cast<std::string>(filename);
  if (m_buffer)
    setbuffer(f, m_buffer, BUFSIZ);
  return true;
}

bool PlainFile::close() {
  invokeFiltersOnClose();
  return closeImpl();
}

bool PlainFile::closeImpl() {
  bool ret = true;
  s_file_data->m_pcloseRet = 0;
  if (!m_closed) {
    if (m_stream) {
      s_file_data->m_pcloseRet = fclose(m_stream);
      m_stream = nullptr;
    } else if (m_fd >= 0) {
      s_file_data->m_pcloseRet = ::close(m_fd);
    }
    if (m_buffer) {
      free(m_buffer);
      m_buffer = nullptr;
    }
    ret = (s_file_data->m_pcloseRet == 0);
    m_closed = true;
    m_fd = -1;
  }
  File::closeImpl();
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// virtual functions

int64_t PlainFile::readImpl(char *buffer, int64_t length) {
  assert(valid());
  assert(length > 0);
  // use read instead of fread to handle EOL in stdin
  size_t ret = ::read(m_fd, buffer, length);
  if (ret == 0
   || (ret == (size_t)-1
    && errno != EWOULDBLOCK && errno != EINTR && errno != EBADF)) {
    m_eof = true;
  }
  return ret == (size_t)-1 ? 0 : ret;
}

int PlainFile::getc() {
  assert(valid());
  return File::getc();
}

// This definition is needed to avoid triggering a gcc compiler error about
// an overloaded virtual when only overriding the one parameter version from
// File.
String PlainFile::read() {
  return File::read();
}

String PlainFile::read(int64_t length) {
  if (length) m_eof = false;
  return File::read(length);
}

int64_t PlainFile::writeImpl(const char *buffer, int64_t length) {
  assert(valid());
  assert(length > 0);

  // use write instead of fwrite to be consistent with read
  // o.w., read-and-write files would not work
  int64_t written = ::write(m_fd, buffer, length);
  return written < 0 ? 0 : written;
}

bool PlainFile::seek(int64_t offset, int whence /* = SEEK_SET */) {
  assert(valid());

  if (whence == SEEK_CUR) {
    off_t result = lseek(m_fd, 0, SEEK_CUR);
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
  // clear the eof flag
  m_eof = false;
  flush();
  // lseek instead of seek to be consistent with read
  off_t result = lseek(m_fd, offset, whence);
  m_position = result;
  return result != (off_t)-1;
}

int64_t PlainFile::tell() {
  assert(valid());
  return m_position;
}

bool PlainFile::eof() {
  assert(valid());
  int64_t avail = m_writepos - m_readpos;
  if (avail > 0) {
    return false;
  }
  return m_eof;
}

bool PlainFile::rewind() {
  assert(valid());
  seek(0);
  m_writepos = 0;
  m_readpos = 0;
  m_position = 0;
  return true;
}

bool PlainFile::stat(struct stat *sb) {
  assert(valid());
  return ::fstat(m_fd, sb) == 0;
}

bool PlainFile::flush() {
  if (m_stream) {
    return fflush(m_stream) == 0;
  }
  assert(valid());
  // No need to flush a file descriptor.
  return true;
}

bool PlainFile::truncate(int64_t size) {
  assert(valid());
  return ftruncate(m_fd, size) == 0;
}

///////////////////////////////////////////////////////////////////////////////
// BuiltinFiles

BuiltinFile::~BuiltinFile() {
  m_closed = true;
  m_stream = nullptr;
  m_fd = -1;
}

bool BuiltinFile::close() {
  invokeFiltersOnClose();
  auto status = ::fclose(m_stream);
  m_closed = true;
  m_stream = nullptr;
  m_fd = -1;
  File::closeImpl();
  return status == 0;
}

void BuiltinFile::sweep() {
  invokeFiltersOnClose();
  // This object was just a wrapper around a FILE* or fd owned by someone else,
  // so don't close it except in explicit calls to close().
  m_stream = nullptr;
  m_fd = -1;
  m_closed = true;
  File::sweep();
}

IMPLEMENT_REQUEST_LOCAL(BuiltinFiles, g_builtin_files);

void BuiltinFiles::requestInit() {
  GetSTDIN();
  GetSTDOUT();
  GetSTDERR();
}

void BuiltinFiles::requestShutdown() {
  m_stdin.releaseForSweep();
  m_stdout.releaseForSweep();
  m_stderr.releaseForSweep();
}

const Variant& BuiltinFiles::GetSTDIN() {
  if (g_builtin_files->m_stdin.isNull()) {
    BuiltinFile *f = NEWOBJ(BuiltinFile)(stdin);
    g_builtin_files->m_stdin = f;
    f->o_setId(1);
    assert(f->o_getId() == 1);
  }
  return g_builtin_files->m_stdin;
}

const Variant& BuiltinFiles::GetSTDOUT() {
  if (g_builtin_files->m_stdout.isNull()) {
    BuiltinFile *f = NEWOBJ(BuiltinFile)(stdout);
    g_builtin_files->m_stdout = f;
    f->o_setId(2);
    assert(f->o_getId() == 2);
  }
  return g_builtin_files->m_stdout;
}

const Variant& BuiltinFiles::GetSTDERR() {
  if (g_builtin_files->m_stderr.isNull()) {
    BuiltinFile *f = NEWOBJ(BuiltinFile)(stderr);
    g_builtin_files->m_stderr = f;
    f->o_setId(3);
    assert(f->o_getId() == 3);
  }
  return g_builtin_files->m_stderr;
}

///////////////////////////////////////////////////////////////////////////////
}
