#ifndef incl_HPHP_CURL_MULTI_RESOURCE_H
#define incl_HPHP_CURL_MULTI_RESOURCE_H

#include "hphp/runtime/ext/extension.h"

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

 private:
  CURLM *m_multi;
  Array m_easyh;
};

/////////////////////////////////////////////////////////////////////////////
}
#endif
