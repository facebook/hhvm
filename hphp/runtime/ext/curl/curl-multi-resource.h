#pragma once

#include "hphp/runtime/ext/extension.h"

#include "hphp/util/compact-vector.h"
#include "hphp/util/type-scan.h"

#include <curl/curl.h>

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////
// CurlMultiResource

struct CurlResource;

struct CurlMultiResource : SweepableResourceData {
  using EasyHandles = CompactVector<req::ptr<CurlResource>>;

  DECLARE_RESOURCE_ALLOCATION(CurlMultiResource)
  CLASSNAME_IS("curl_multi")
  const String& o_getClassNameHook() const override { return classnameof(); }
  bool isInvalid() const override { return !m_multi; }

  CurlMultiResource();
  ~CurlMultiResource() { close(); }
  void close();

  bool setOption(int option, const Variant& value);
  const EasyHandles& getEasyHandles() const { return m_easyh; }

  CURLMcode add(CurlResource* curle);
  CURLMcode remove(CurlResource* curle, bool leak = false);
  OptResource find(CURL *cp);

  CURLM* get();
  void check_exceptions();

  void setInExec(bool b);
  bool anyInExec() const;

 private:
  void removeEasyHandles(bool leak = false);

  CURLM *m_multi;
  TYPE_SCAN_IGNORE_FIELD(m_multi);
  EasyHandles m_easyh;
};

/////////////////////////////////////////////////////////////////////////////
}
