/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_CURL_SHARE_RESOURCE_H
#define incl_HPHP_CURL_SHARE_RESOURCE_H

#include "hphp/runtime/ext/extension.h"
#include "hphp/util/type-scan.h"
#include <curl/curl.h>

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

struct CurlShareResource : SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(CurlShareResource)
  CLASSNAME_IS("curl_share")
  const String& o_getClassNameHook() const override { return classnameof(); }
  bool isInvalid() const override { return !m_share; }

  CurlShareResource();
  ~CurlShareResource() { close(); }
  void close();
  bool setOption(int option, const Variant& value);
  static bool isLongOption(long option);
  bool setLongOption(long option, long value);
  CURLcode attachToCurlHandle(CURL *cp);
 private:
  CURLSH* m_share;
  // CURLSH is a typedef to void
  TYPE_SCAN_IGNORE_FIELD(m_share);
};

/////////////////////////////////////////////////////////////////////////////
}
#endif
