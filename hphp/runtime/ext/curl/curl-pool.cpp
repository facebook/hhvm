#include "hphp/runtime/ext/curl/curl-pool.h"

#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/lock.h"
#include "hphp/util/timer.h"

#include <folly/portability/Time.h>

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////
// PooledCurlHandle

PooledCurlHandle::~PooledCurlHandle() {
  if (m_handle != nullptr) {
    curl_easy_cleanup(m_handle);
  }
}

CURL* PooledCurlHandle::useHandle() {
  if (m_handle == nullptr) {
    m_handle = curl_easy_init();
  }

  if (m_connRecycleAfter > 0 &&
      m_numUsages % m_connRecycleAfter == 0) {
    curl_easy_cleanup(m_handle);
    m_handle = curl_easy_init();
    m_numUsages = 0;
  }

  m_numUsages++;
  return m_handle;
}

void PooledCurlHandle::resetHandle() {
  if (m_handle != nullptr) {
    curl_easy_reset(m_handle);
  }
}

/////////////////////////////////////////////////////////////////////////////
// CurlHandlePool

CurlHandlePool::CurlHandlePool(int size,
                               int connGetTimeout,
                               int reuseLimit)
: m_size(size),
  m_connGetTimeout(connGetTimeout),
  m_reuseLimit(reuseLimit) {
  for (int i = 0; i < size; i++) {
    m_handleStack.push(new PooledCurlHandle(reuseLimit));
  }
  pthread_cond_init(&m_cond, nullptr);
}

CurlHandlePool::~CurlHandlePool() {
  Lock lock(m_mutex);
  while (!m_handleStack.empty()) {
    PooledCurlHandle *handle = m_handleStack.top();
    m_handleStack.pop();
    delete handle;
  }
}

PooledCurlHandle* CurlHandlePool::fetch() {
  struct timespec ts;
  gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += m_connGetTimeout / 1000;
  ts.tv_nsec += 1000000 * (m_connGetTimeout % 1000);

  Lock lock(m_mutex);
  m_statsFetches += 1;
  // wait until the user-specified timeout for an available handle
  if (m_handleStack.empty()) {
    m_statsEmpty += 1;
    do {
      if (ETIMEDOUT == pthread_cond_timedwait(&m_cond, &m_mutex.getRaw(),
                                              &ts)
      ) {
        m_statsFetchUs += m_connGetTimeout * 1000;
        SystemLib::throwRuntimeExceptionObject(
          "Timeout reached waiting for an available pooled curl connection!");
      }
    } while (m_handleStack.empty());
  }

  PooledCurlHandle* ret = m_handleStack.top();
  assertx(ret);
  m_handleStack.pop();

  struct timespec after;
  gettime(CLOCK_REALTIME, &after);
  m_statsFetchUs += m_connGetTimeout * 1000 - gettime_diff_us(after, ts);

  return ret;
}

void CurlHandlePool::store(PooledCurlHandle* handle) {
  Lock lock(m_mutex);
  handle->resetHandle();
  m_handleStack.push(handle);
  pthread_cond_signal(&m_cond);
}

ReadWriteMutex CurlHandlePool::namedPoolsMutex;
std::map<std::string, CurlHandlePoolPtr> CurlHandlePool::namedPools;

/////////////////////////////////////////////////////////////////////////////
}
