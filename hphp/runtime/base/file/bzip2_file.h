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

#ifndef incl_HPHP_BZIP2_FILE_H_
#define incl_HPHP_BZIP2_FILE_H_

#include <runtime/base/base_includes.h>
#include <runtime/base/file/plain_file.h>
#include <stdio.h>
#include <bzlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// BZ2File class

class BZ2File : public File {
public:
  DECLARE_OBJECT_ALLOCATION(BZ2File);

  static StaticString s_class_name;
  // overriding ResourceData
  CStrRef o_getClassNameHook() const { return s_class_name; }

  BZ2File();
  BZ2File(PlainFile* innerFile);
  virtual ~BZ2File();

  bool open(CStrRef filename, CStrRef mode);
  bool close();
  int64_t errnu();
  String errstr();
  Variant error();
  virtual bool flush();
  virtual int64_t readImpl(char * buf, int64_t length);
  virtual int64_t writeImpl(const char * buf, int64_t length);
  virtual bool eof();

private:
  BZFILE * m_bzFile;
  PlainFile * m_innerFile;
  bool m_eof;
  bool closeImpl();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_BZIP2_FILE_H_
