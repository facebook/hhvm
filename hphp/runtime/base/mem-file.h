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

#ifndef incl_HPHP_MEM_FILE_H_
#define incl_HPHP_MEM_FILE_H_

#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/complex-types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * memory based files.
 */
class MemFile : public File {
public:
  DECLARE_RESOURCE_ALLOCATION(MemFile);

  MemFile();
  MemFile(const char *data, int64_t len);
  virtual ~MemFile();

  CLASSNAME_IS("MemFile");
  // overriding ResourceData
  const String& o_getClassNameHook() const { return classnameof(); }

  virtual bool open(const String& filename, const String& mode);
  virtual bool close();
  virtual int64_t readImpl(char *buffer, int64_t length);
  virtual int getc();
  virtual int64_t writeImpl(const char *buffer, int64_t length);
  virtual bool seekable() { return true;}
  virtual bool seek(int64_t offset, int whence = SEEK_SET);
  virtual int64_t tell();
  virtual bool eof();
  virtual bool rewind();
  virtual bool flush();

  void unzip();

protected:
  std::string m_name; // name of the memory file
  char *m_data;       // data of the memory file
  int64_t m_len;        // length of the memory file
  int64_t m_cursor;     // m_data's read position
  bool m_malloced;    // whether to free m_data on delete

  bool closeImpl();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_MEM_FILE_H_
