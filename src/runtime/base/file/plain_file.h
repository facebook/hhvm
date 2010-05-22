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

#ifndef __HPHP_PLAIN_FILE_H__
#define __HPHP_PLAIN_FILE_H__

#include <runtime/base/file/file.h>
#include <runtime/base/execution_context.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define STDIN  (BuiltinFiles::getSTDIN())
#define STDOUT (BuiltinFiles::getSTDOUT())
#define STDERR (BuiltinFiles::getSTDERR())

/**
 * A file system file that's nothing but ordinary. A simple FILE* wrapper.
 */
class PlainFile : public File {
public:
  DECLARE_OBJECT_ALLOCATION(PlainFile);

  PlainFile(FILE *stream = NULL, bool pipe = false);
  virtual ~PlainFile();

  // overriding ResourceData
  const char *o_getClassName() const { return "PlainFile";}

  // implementing File
  virtual bool open(CStrRef filename, CStrRef mode);
  virtual bool close();
  virtual int readImpl(char *buffer, int length);
  virtual int getc();
  virtual int writeImpl(const char *buffer, int length);
  virtual bool seek(int offset, int whence = SEEK_SET);
  virtual int tell();
  virtual bool eof();
  virtual bool rewind();
  virtual bool flush();
  virtual bool truncate(int size);

  FILE *getStream() { return m_stream;}

  static CVarRef getStdIn();
  static CVarRef getStdOut();
  static CVarRef getStdErr();

protected:
  FILE *m_stream;
  bool m_eof;

  bool closeImpl();
};

/**
 * A request-local wrapper for the three standard files: 
 * STDIN, STDOUT, and STDERR.
 */
class BuiltinFiles : public RequestEventHandler {
public:
  virtual void requestInit() { }
  virtual void requestShutdown();

  static CVarRef getSTDIN();
  static CVarRef getSTDOUT();
  static CVarRef getSTDERR();

private:
  Variant m_stdin;
  Variant m_stdout;
  Variant m_stderr;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_PLAIN_FILE_H__
