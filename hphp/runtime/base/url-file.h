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

#ifndef incl_HPHP_URL_FILE_H_
#define incl_HPHP_URL_FILE_H_

#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/string-buffer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * url based files.
 */
class UrlFile : public MemFile {
public:
  DECLARE_RESOURCE_ALLOCATION(UrlFile);

  explicit UrlFile(const char *method = "GET", CArrRef headers = null_array,
                   const String& postData = null_string, int maxRedirect = 20,
                   int timeout = -1);

  // overriding ResourceData
  const String& o_getClassNameHook() const { return classnameof(); }

  virtual bool open(const String& filename, const String& mode);
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
