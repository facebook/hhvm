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

#ifndef __HPHP_OUTPUT_FILE_H__
#define __HPHP_OUTPUT_FILE_H__

#include <runtime/base/file/file.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * For php://output, a simple wrapper of g_context->out().
 */
class OutputFile : public File {
public:
  DECLARE_OBJECT_ALLOCATION(OutputFile);

  OutputFile(CStrRef filename);
  virtual ~OutputFile();

  bool valid() const { return !m_closed;}

  static StaticString s_class_name;
  // overriding ResourceData
  CStrRef o_getClassName() const { return s_class_name; }

  // implementing File
  virtual bool open(CStrRef filename, CStrRef mode);
  virtual bool close();
  virtual int64 readImpl(char *buffer, int64 length);
  virtual int getc();
  virtual int64 writeImpl(const char *buffer, int64 length);
  virtual bool seek(int64 offset, int whence = SEEK_SET);
  virtual int64 tell();
  virtual bool eof();
  virtual bool rewind();
  virtual bool flush();
  virtual bool truncate(int64 size);

protected:
  bool closeImpl();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_OUTPUT_FILE_H__
