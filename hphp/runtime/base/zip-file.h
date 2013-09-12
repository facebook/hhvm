/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_ZIP_FILE_H_
#define incl_HPHP_ZIP_FILE_H_

#include "hphp/runtime/base/plain-file.h"
#include <zlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * zlib based files.
 */
class ZipFile : public File {
public:
  DECLARE_RESOURCE_ALLOCATION(ZipFile);

  ZipFile();
  virtual ~ZipFile();

  static StaticString s_class_name;
  // overriding ResourceData
  CStrRef o_getClassNameHook() const { return s_class_name; }

  virtual bool open(CStrRef filename, CStrRef mode);
  virtual bool close();
  virtual int64_t readImpl(char *buffer, int64_t length);
  virtual int64_t writeImpl(const char *buffer, int64_t length);
  virtual bool seekable() { return true;}
  virtual bool seek(int64_t offset, int whence = SEEK_SET);
  virtual int64_t tell();
  virtual bool eof();
  virtual bool rewind();
  virtual bool flush();

private:
  gzFile m_gzFile;
  PlainFile *m_innerFile;

  bool closeImpl();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ZIP_FILE_H_
