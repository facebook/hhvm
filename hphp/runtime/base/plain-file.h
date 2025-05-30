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

#pragma once

#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/request-event-handler.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * A file system file that's nothing but ordinary. A simple FILE* wrapper.
 */
struct PlainFile : File {
  DECLARE_RESOURCE_ALLOCATION(PlainFile)

  explicit PlainFile(FILE *stream = nullptr,
                     bool nonblocking = false,
                     const String& wrapper_type = null_string,
                     const String& stream_type = null_string);
  explicit PlainFile(int fd,
                     bool nonblocking = false,
                     const String& wrapper = null_string,
                     const String& stream_type = null_string);
  ~PlainFile() override;

  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  // implementing File
  bool open(const String& filename, const String& mode) override;
  bool close(int* unused = nullptr) override;
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
};

/**
 * This is wrapper for fds that cannot be closed.
 */
struct BuiltinFile : PlainFile {
  explicit BuiltinFile(FILE *stream);
  explicit BuiltinFile(int fd);
  ~BuiltinFile() override;
  bool close(int* unused = nullptr) final;
  void sweep() override;
};
static_assert(sizeof(BuiltinFile) == sizeof(PlainFile),
              "BuiltinFile inherits PlainFile::heapSize()");

/**
 * A request-local wrapper for the three standard files:
 * STDIN, STDOUT, and STDERR.
 */
struct BuiltinFiles final : RequestEventHandler {
  static const Variant& getSTDIN();
  static const Variant& getSTDOUT();
  static const Variant& getSTDERR();
  static Variant getSTDIN(const StringData* name);
  static Variant getSTDOUT(const StringData* name);
  static Variant getSTDERR(const StringData* name);

  void requestInit() override;
  void requestShutdown() override;

private:
  Variant m_stdin;
  Variant m_stdout;
  Variant m_stderr;
};

void clearThreadLocalIO();
void setThreadLocalIO(FILE* in, FILE* out, FILE* err);

///////////////////////////////////////////////////////////////////////////////
}

