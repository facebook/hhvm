#pragma once

#include "hphp/runtime/ext/extension.h"

#include "hphp/util/type-scan.h"

#include <curl/curl.h>

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////
// CurlMultiResource

struct CurlResource;

struct CurlMultiResource : SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(CurlMultiResource)
  CLASSNAME_IS("curl_multi")
  const String& o_getClassNameHook() const override { return classnameof(); }
  bool isInvalid() const override { return !m_multi; }

  CurlMultiResource();
  ~CurlMultiResource() { close(); }
  void close();

  bool setOption(int option, const Variant& value);
  void add(const Resource& ch) { m_easyh.append(ch); }
  const Array& getEasyHandles() const { return m_easyh; }

  void remove(req::ptr<CurlResource> curle);
  Resource find(CURL *cp);

  CURLM* get();
  void check_exceptions();

  void setInExec(bool b);
  bool anyInExec() const;

 private:
  CURLM *m_multi;
  // CURLM is a typedef to void
  TYPE_SCAN_IGNORE_FIELD(m_multi);
  Array m_easyh;
};

/////////////////////////////////////////////////////////////////////////////
}
