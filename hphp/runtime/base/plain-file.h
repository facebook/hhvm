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

#ifndef incl_HPHP_PLAIN_FILE_H_
#define incl_HPHP_PLAIN_FILE_H_

#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/request-event-handler.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * A file system file that's nothing but ordinary. A simple FILE* wrapper.
 */
class PlainFile : public File {
public:
  DECLARE_RESOURCE_ALLOCATION(PlainFile);

  explicit PlainFile(FILE *stream = nullptr,
                     bool nonblocking = false,
                     const String& wrapper_type = null_string,
                     const String& stream_type = null_string);
  explicit PlainFile(int fd,
                     bool nonblocking = false,
                     const String& wrapper = null_string,
                     const String& stream_type = null_string);
  virtual ~PlainFile();

  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  // implementing File
  bool open(const String& filename, const String& mode) override;
  bool close() override;
  int64_t readImpl(char *buffer, int64_t length) override;
  int getc() override;
  String read() override;
  String read(int64_t length) override;
  int64_t writeImpl(const char *buffer, int64_t length) override;
  bool seekable() override { return true;}
  bool seek(int64_t offset, int whence = SEEK_SET) override;
  int64_t tell() override;
  bool eof() override;
  bool rewind() override;
  bool flush() override;
  bool truncate(int64_t size) override;
  bool stat(struct stat *sb) override;

  FILE *getStream() { return m_stream;}

protected:
  FILE *m_stream;
  char *m_buffer;       // For setbuffer.  Needed to reduce mmap
                        // contention due to how glibc allocates memory
                        // for buffered io.

  bool closeImpl();
};

/**
 * This is wrapper for fds that cannot be closed.
 */
struct BuiltinFile : PlainFile {
  explicit BuiltinFile(FILE *stream) : PlainFile(stream, true) {}
  explicit BuiltinFile(int fd) : PlainFile(fd, true) {}
  virtual ~BuiltinFile();
  bool close() override;
  void sweep() override;
};
static_assert(sizeof(BuiltinFile) == sizeof(PlainFile),
              "BuiltinFile inherits PlainFile::heapSize()");

/**
 * A request-local wrapper for the three standard files:
 * STDIN, STDOUT, and STDERR.
 */
struct BuiltinFiles final : RequestEventHandler {
  void requestInit() override;
  void requestShutdown() override;

  static const Variant& GetSTDIN();
  static const Variant& GetSTDOUT();
  static const Variant& GetSTDERR();

private:
  Variant m_stdin;
  Variant m_stdout;
  Variant m_stderr;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_PLAIN_FILE_H_
