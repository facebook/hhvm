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

#ifndef incl_HPHP_MEM_FILE_H_
#define incl_HPHP_MEM_FILE_H_

#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * memory based files.
 */
class MemFile : public File {
public:
  DECLARE_RESOURCE_ALLOCATION(MemFile);

  explicit MemFile(const String& wrapper_type = null_string,
                   const String& stream_type = empty_string_ref);
  MemFile(const char *data, int64_t len,
          const String& wrapper_type = null_string,
          const String& stream_type = empty_string_ref);
  virtual ~MemFile();

  CLASSNAME_IS("MemFile");
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  bool open(const String& filename, const String& mode) override;
  bool close() override;
  int64_t readImpl(char *buffer, int64_t length) override;
  int getc() override;
  int64_t writeImpl(const char *buffer, int64_t length) override;
  bool seekable() override { return true;}
  bool seek(int64_t offset, int whence = SEEK_SET) override;
  int64_t tell() override;
  bool eof() override;
  bool rewind() override;
  bool flush() override;

  Array getMetaData() override;

  void unzip();

protected:
  char *m_data;       // data of the memory file
  int64_t m_len;      // length of the memory file
  int64_t m_cursor;   // m_data's read position
  bool m_malloced;    // whether to free m_data on delete

  bool closeImpl();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_MEM_FILE_H_
