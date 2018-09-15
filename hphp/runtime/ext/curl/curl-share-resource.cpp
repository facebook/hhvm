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

#include "hphp/runtime/ext/curl/curl-share-resource.h"
#include "hphp/runtime/ext/curl/curl-resource.h"
#include "hphp/runtime/base/builtin-functions.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

CurlShareResource::CurlShareResource() {
  m_share = curl_share_init();
}

void CurlShareResource::close() {
  if (m_share) {
    curl_share_cleanup(m_share);
    m_share = nullptr;
  }
}

void CurlShareResource::sweep() {
  close();
}

bool CurlShareResource::setOption(int option, const Variant& value) {
  if (m_share == nullptr) {
    raise_warning("curl_share_setopt():"
                  " curl share not created or is already closed");
    return false;
  }

  bool ret;
  if (isLongOption(option)) {
    ret = setLongOption(option, value.toInt64());
  } else {
    raise_warning("curl_share_setopt():"
                  "Invalid curl share configuration option");
    ret = false;
  }
  return ret;
}

bool CurlShareResource::isLongOption(long option) {
  switch (option) {
    case CURLSHOPT_SHARE:
    case CURLSHOPT_UNSHARE:
      return true;
    default:
      return false;
  }
}

bool CurlShareResource::setLongOption(long option, long value) {
  CURLSHcode error = CURLSHE_OK;
  error = curl_share_setopt(m_share,
                            (CURLSHoption)option,
                            value);
  return error == CURLSHE_OK;
}

CURLcode CurlShareResource::attachToCurlHandle(CURL *cp) {
  return curl_easy_setopt(cp, CURLOPT_SHARE, m_share);
}

/////////////////////////////////////////////////////////////////////////////
}
