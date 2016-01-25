#include "hphp/runtime/ext/curl/curl-multi-resource.h"
#include "hphp/runtime/ext/curl/curl-resource.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"

#include <curl/easy.h>
#include <curl/multi.h>

namespace {
const HPHP::StaticString
  s_exception("exception"),
  s_previous("previous");
}

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

CurlMultiResource::CurlMultiResource() {
  m_multi = curl_multi_init();
}

void CurlMultiResource::close() {
  if (m_multi) {
    curl_multi_cleanup(m_multi);
    m_easyh.clear();
    m_multi = nullptr;
  }
}

void CurlMultiResource::sweep() {
  if (m_multi) {
    curl_multi_cleanup(m_multi);
  }
}

bool CurlMultiResource::setOption(int option, const Variant& value) {
#if LIBCURL_VERSION_NUM <= 0x070f04 /* 7.15.4 */
  return false;
#endif
  if (m_multi == nullptr) {
    return false;
  }

  CURLMcode error = CURLM_OK;
  switch (option) {
#if LIBCURL_VERSION_NUM >= 0x071000 /* 7.16.0 */
    case CURLMOPT_PIPELINING:
#if LIBCURL_VERSION_NUM >= 0x071003 /* 7.16.3 */
    case CURLMOPT_MAXCONNECTS:
#endif
      error = curl_multi_setopt(m_multi,
                                (CURLMoption)option,
                                value.toInt64());
      break;
#endif

    default:
      raise_warning("curl_multi_setopt():"
                    "Invalid curl multi configuration option");
      error = CURLM_UNKNOWN_OPTION;
      break;
  }

  return error == CURLM_OK;
}

void CurlMultiResource::remove(req::ptr<CurlResource> curle) {
  for (ArrayIter iter(m_easyh); iter; ++iter) {
    if (cast<CurlResource>(iter.second())->get(true) ==
        curle->get()) {
      m_easyh.remove(iter.first());
      return;
    }
  }
}

Resource CurlMultiResource::find(CURL *cp) {
  for (ArrayIter iter(m_easyh); iter; ++iter) {
    if (cast<CurlResource>(iter.second())->get(true) == cp) {
      return iter.second().toResource();
    }
  }
  return Resource();
}

void CurlMultiResource::check_exceptions() {
  Exception* cppException = nullptr;
  Object phpException;
  for (ArrayIter iter(m_easyh); iter; ++iter) {
    auto curl = cast<CurlResource>(iter.second());
    CurlResource::ExceptionType nextException(curl->getAndClearException());
    if (!nextException) continue;
    if (CurlResource::isPhpException(nextException)) {
      Object e(CurlResource::getPhpException(nextException));
      e->o_set(s_previous, phpException, s_exception);
      phpException = std::move(e);
    } else {
      auto e = CurlResource::getCppException(nextException);
      if (auto f = dynamic_cast<FatalErrorException*>(e)) {
        if (!f->isRecoverable()) f->throwException();
      }
      delete cppException;
      cppException = e;
    }
  }
  if (cppException) cppException->throwException();
  if (!phpException.isNull()) throw_object(phpException);
}

CURLM* CurlMultiResource::get() {
  if (m_multi == nullptr) {
    throw_null_pointer_exception();
  }
  return m_multi;
}

/////////////////////////////////////////////////////////////////////////////
}
