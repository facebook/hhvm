/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_URL_FILE_H_
#define incl_HPHP_URL_FILE_H_

#include "hphp/runtime/base/mem_file.h"
#include "hphp/runtime/base/string_buffer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * url based files.
 */
class UrlFile : public MemFile {
public:
  DECLARE_OBJECT_ALLOCATION(UrlFile);

  explicit UrlFile(const char *method = "GET", CArrRef headers = null_array,
                   CStrRef postData = null_string, int maxRedirect = 20,
                   int timeout = -1);

  static StaticString s_class_name;
  // overriding ResourceData
  CStrRef o_getClassNameHook() const { return s_class_name; }

  virtual bool open(CStrRef filename, CStrRef mode);
  virtual int64_t writeImpl(const char *buffer, int64_t length);
  virtual bool flush();
  virtual Array getWrapperMetaData() { return m_responseHeaders; }
  String getLastError();

private:
  bool m_get;
  Array m_headers;
  String m_postData;
  int m_maxRedirect;
  int m_timeout;
  std::string m_error;
  StringBuffer m_response;
  Array m_responseHeaders;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_URL_FILE_H_
