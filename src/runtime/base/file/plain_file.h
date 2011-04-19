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

#ifndef __HPHP_PLAIN_FILE_H__
#define __HPHP_PLAIN_FILE_H__

#include <runtime/base/file/file.h>
#include <runtime/base/execution_context.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * A file system file that's nothing but ordinary. A simple FILE* wrapper.
 */
class PlainFile : public File {
public:
  DECLARE_OBJECT_ALLOCATION(PlainFile);

  PlainFile(FILE *stream = NULL, bool nonblocking = false);
  PlainFile(int fd, bool nonblocking = false);
  virtual ~PlainFile();

  static StaticString s_class_name;
  // overriding ResourceData
  CStrRef o_getClassName() const { return s_class_name; }

  // implementing File
  virtual bool open(CStrRef filename, CStrRef mode);
  virtual bool close();
  virtual int64 readImpl(char *buffer, int64 length);
  virtual int getc();
  virtual int64 writeImpl(const char *buffer, int64 length);
  virtual bool seekable() { return true;}
  virtual bool seek(int64 offset, int whence = SEEK_SET);
  virtual int64 tell();
  virtual bool eof();
  virtual bool rewind();
  virtual bool flush();
  virtual bool truncate(int64 size);

  FILE *getStream() { return m_stream;}
  virtual const char *getStreamType() const { return "STDIO";}

  static CVarRef getStdIn();
  static CVarRef getStdOut();
  static CVarRef getStdErr();

protected:
  FILE *m_stream;
  bool m_eof;

  bool closeImpl();
};

/**
 * This is wrapper for fds that cannot be closed.
 */
class BuiltinFile : public PlainFile {
public:
  BuiltinFile(FILE *stream) : PlainFile(stream, true) {}
  virtual ~BuiltinFile();
  virtual bool close();
};

/**
 * A request-local wrapper for the three standard files:
 * STDIN, STDOUT, and STDERR.
 */
class BuiltinFiles : public RequestEventHandler {
public:
  virtual void requestInit();
  virtual void requestShutdown();

  static CVarRef GetSTDIN();
  static CVarRef GetSTDOUT();
  static CVarRef GetSTDERR();

private:
  Variant m_stdin;
  Variant m_stdout;
  Variant m_stderr;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_PLAIN_FILE_H__
