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

#ifndef __HPHP_URL_FILE_H__
#define __HPHP_URL_FILE_H__

#include <runtime/base/file/mem_file.h>
#include <runtime/base/util/string_buffer.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * url based files.
 */
class UrlFile : public MemFile {
public:
  DECLARE_OBJECT_ALLOCATION(UrlFile);

  UrlFile(const char *method = "GET", CArrRef headers = null_array,
          CStrRef postData = null_string, int maxRedirect = 20,
          int timeout = -1);

  // overriding ResourceData
  const char *o_getClassName() const { return "UrlFile";}

  virtual bool open(CStrRef filename, CStrRef mode);
  virtual int writeImpl(const char *buffer, int length);
  virtual bool flush();

private:
  bool m_get;
  Array m_headers;
  String m_postData;
  int m_maxRedirect;
  int m_timeout;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_URL_FILE_H__
