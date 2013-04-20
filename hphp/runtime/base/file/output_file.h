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

#ifndef incl_HPHP_OUTPUT_FILE_H_
#define incl_HPHP_OUTPUT_FILE_H_

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
  CStrRef o_getClassNameHook() const { return s_class_name; }

  // implementing File
  virtual bool open(CStrRef filename, CStrRef mode);
  virtual bool close();
  virtual int64_t readImpl(char *buffer, int64_t length);
  virtual int getc();
  virtual int64_t writeImpl(const char *buffer, int64_t length);
  virtual bool seek(int64_t offset, int whence = SEEK_SET);
  virtual int64_t tell();
  virtual bool eof();
  virtual bool rewind();
  virtual bool flush();
  virtual bool truncate(int64_t size);

protected:
  bool closeImpl();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_OUTPUT_FILE_H_
