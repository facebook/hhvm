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

#include <unistd.h>
#include <runtime/base/file/plain_file.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/util/request_local.h>

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION(PlainFile)

///////////////////////////////////////////////////////////////////////////////

StaticString PlainFile::s_class_name("PlainFile");

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

PlainFile::PlainFile(FILE *stream, bool nonblocking)
  : File(nonblocking), m_stream(stream), m_eof(false) {
  if (stream) m_fd = fileno(stream);
}

PlainFile::PlainFile(int fd, bool nonblocking)
  : File(nonblocking), m_stream(NULL), m_eof(false) {
  m_fd = fd;
}

PlainFile::~PlainFile() {
  closeImpl();
}

bool PlainFile::open(CStrRef filename, CStrRef mode) {
  int fd;
  FILE *f;
  ASSERT(m_stream == NULL);
  ASSERT(m_fd == -1);

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
  return true;
}

bool PlainFile::close() {
  return closeImpl();
}

bool PlainFile::closeImpl() {
  bool ret = true;
  s_file_data->m_pcloseRet = 0;
  if (!m_closed) {
    if (m_stream) {
      s_file_data->m_pcloseRet = fclose(m_stream);
      m_stream = NULL;
    } else if (m_fd >= 0) {
      s_file_data->m_pcloseRet = ::close(m_fd);
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

int64 PlainFile::readImpl(char *buffer, int64 length) {
  ASSERT(valid());
  ASSERT(length > 0);
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
  ASSERT(valid());
  return File::getc();
}

int64 PlainFile::writeImpl(const char *buffer, int64 length) {
  ASSERT(valid());
  ASSERT(length > 0);

  // use write instead of fwrite to be consistent with read
  // o.w., read-and-write files would not work
  int64 written = ::write(m_fd, buffer, length);
  return written < 0 ? 0 : written;
}

bool PlainFile::seek(int64 offset, int whence /* = SEEK_SET */) {
  ASSERT(valid());

  if (whence == SEEK_CUR) {
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

int64 PlainFile::tell() {
  ASSERT(valid());
  return m_position;
}

bool PlainFile::eof() {
  ASSERT(valid());
  int64 avail = m_writepos - m_readpos;
  if (avail > 0) {
    return false;
  }
  return m_eof;
}

bool PlainFile::rewind() {
  ASSERT(valid());
  seek(0);
  m_writepos = 0;
  m_readpos = 0;
  m_position = 0;
  return true;
}

bool PlainFile::flush() {
  if (m_stream) {
    return fflush(m_stream) == 0;
  }
  ASSERT(valid());
  // No need to flush a file descriptor.
  return true;
}

bool PlainFile::truncate(int64 size) {
  ASSERT(valid());
  return ftruncate(m_fd, size) == 0;
}

///////////////////////////////////////////////////////////////////////////////
// BuiltinFiles

BuiltinFile::~BuiltinFile() {
  m_closed = true;
  m_stream = NULL;
  m_fd = -1;
}

bool BuiltinFile::close() {
  m_closed = true;
  m_stream = NULL;
  m_fd = -1;
  File::closeImpl();
  return true;
}

IMPLEMENT_REQUEST_LOCAL(BuiltinFiles, g_builtin_files);

void BuiltinFiles::requestInit() {
  GetSTDIN();
  GetSTDOUT();
  GetSTDERR();
}

void BuiltinFiles::requestShutdown() {
  m_stdin.reset();
  m_stdout.reset();
  m_stderr.reset();
}

CVarRef BuiltinFiles::GetSTDIN() {
  if (g_builtin_files->m_stdin.isNull()) {
    BuiltinFile *f = NEWOBJ(BuiltinFile)(stdin);
    g_builtin_files->m_stdin = f;
    f->o_setId(1);
    ASSERT(f->o_getId() == 1);
  }
  return g_builtin_files->m_stdin;
}

CVarRef BuiltinFiles::GetSTDOUT() {
  if (g_builtin_files->m_stdout.isNull()) {
    BuiltinFile *f = NEWOBJ(BuiltinFile)(stdout);
    g_builtin_files->m_stdout = f;
    f->o_setId(2);
    ASSERT(f->o_getId() == 2);
  }
  return g_builtin_files->m_stdout;
}

CVarRef BuiltinFiles::GetSTDERR() {
  if (g_builtin_files->m_stderr.isNull()) {
    BuiltinFile *f = NEWOBJ(BuiltinFile)(stderr);
    g_builtin_files->m_stderr = f;
    f->o_setId(3);
    ASSERT(f->o_getId() == 3);
  }
  return g_builtin_files->m_stderr;
}

///////////////////////////////////////////////////////////////////////////////
}
