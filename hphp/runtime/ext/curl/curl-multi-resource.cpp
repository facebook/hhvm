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
  if (!m_multi) return;
  removeEasyHandles();
  curl_multi_cleanup(m_multi);
  m_multi = nullptr;
}

void CurlMultiResource::sweep() {
  if (!m_multi) return;
  removeEasyHandles(true);
  curl_multi_cleanup(m_multi);
}

void CurlMultiResource::removeEasyHandles(bool leak) {
  auto index_to_remove = 0;
  auto const size = m_easyh.size();
  for (auto i = 0; i < size; i++) {
    assertx(index_to_remove < m_easyh.size());
    auto const code = remove(m_easyh[index_to_remove].get(), leak);
    if (code != CURLM_OK) index_to_remove++;
  }
}

bool CurlMultiResource::setOption(int option, const Variant& value) {
#if LIBCURL_VERSION_NUM <= 0x070f04 /* 7.15.4 */
  return false;
#endif
  if (!m_multi) {
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

CURLMcode CurlMultiResource::add(CurlResource* curle) {
  // Don't add the handle to our metadata if the add op fails.
  if (!m_multi) return CURLM_BAD_HANDLE;
  if (!curle->get()) return CURLM_BAD_EASY_HANDLE;
  if (curle->m_multi) return CURLM_ADDED_ALREADY;
  auto const code = curl_multi_add_handle(m_multi, curle->get());
  if (code != CURLM_OK) return code;

  curle->m_multi = this;
  m_easyh.emplace_back(curle);
  return CURLM_OK;
}

CURLMcode CurlMultiResource::remove(CurlResource* curle, bool leak) {
  // Don't remove the handle from our metadata if the remove op fails.
  if (!m_multi) return CURLM_BAD_HANDLE;
  if (!curle->get()) return CURLM_BAD_EASY_HANDLE;
  if (curle->m_multi != this) return CURLM_OK; // we do this sometimes...
  auto const code = curl_multi_remove_handle(m_multi, curle->get());
  if (code != CURLM_OK) return code;

  // Find the index to remove.
  auto const index_to_remove = [&]{
    for (auto i = 0; i < m_easyh.size(); i++) {
      if (m_easyh[i].get() == curle) return i;
    }
    always_assert(false);
  }();

  // Remove the easy handle, leaking it if requested.
  curle->m_multi = nullptr;
  if (leak) m_easyh[index_to_remove].detach();
  auto const last = m_easyh.size() - 1;
  if (index_to_remove != last) {
    m_easyh[index_to_remove] = std::move(m_easyh[last]);
  }
  m_easyh.pop_back();
  return CURLM_OK;
}

OptResource CurlMultiResource::find(CURL *cp) {
  for (auto const& curl : m_easyh) {
    if (curl->get() == cp) return OptResource(curl.get());
  }
  return OptResource();
}

void CurlMultiResource::setInExec(bool b) {
  for (auto& curl : m_easyh) {
    curl->m_in_exec = b;
  }
}

bool CurlMultiResource::anyInExec() const {
  for (auto const& curl : m_easyh) {
    if (curl->m_in_exec) return true;
  }
  return false;
}

void CurlMultiResource::check_exceptions() {
  SCOPE_EXIT {
    if (debug) {
      for (auto const& curl : m_easyh) {
        always_assert(!curl->m_exception);
      }
    }
  };

  // If we exit unexpectedly, ensure we've released any queued exceptions.
  SCOPE_EXIT {
    for (auto const& curl : m_easyh) {
      if (auto const exn = curl->getAndClearException()) {
        if (!CurlResource::isPhpException(exn)) {
          delete CurlResource::getCppException(exn);
        }
      }
    }
  };

  Exception* cppException = nullptr;
  Object phpException;
  for (auto const& curl : m_easyh) {
    auto const nextException = curl->getAndClearException();
    if (!nextException) continue;
    if (CurlResource::isPhpException(nextException)) {
      Object e(CurlResource::getPhpException(nextException));
      e->o_set(s_previous, phpException, s_exception);
      phpException = std::move(e);
    } else {
      auto const e = CurlResource::getCppException(nextException);
      if (auto const f = dynamic_cast<FatalErrorException*>(e)) {
        if (!f->isRecoverable()) {
          delete cppException;
          f->throwException();
        }
      }
      delete cppException;
      cppException = e;
    }
  }
  if (cppException) cppException->throwException();
  if (!phpException.isNull()) throw_object(phpException);
}

CURLM* CurlMultiResource::get() {
  if (!m_multi) {
    throw_null_pointer_exception();
  }
  return m_multi;
}

/////////////////////////////////////////////////////////////////////////////
}
