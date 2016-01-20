#include "hphp/runtime/ext/curl/curl-pool.h"

#include "hphp/runtime/base/runtime-error.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/lock.h"

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

CurlHandlePool::CurlHandlePool(int poolSize,
                               int waitTimeout,
                               int numConnReuses)
: m_waitTimeout(waitTimeout) {
  for (int i = 0; i < poolSize; i++) {
    m_handleStack.push(new PooledCurlHandle(numConnReuses));
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
  Lock lock(m_mutex);

  // wait until the user-specified timeout for an available handle
  struct timespec ts;
  gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += m_waitTimeout / 1000;
  ts.tv_nsec += 1000000 * (m_waitTimeout % 1000);
  while (m_handleStack.empty()) {
    if (ETIMEDOUT == pthread_cond_timedwait(&m_cond, &m_mutex.getRaw(), &ts)) {
      raise_error("Timeout reached waiting for an "
                  "available pooled curl connection!");
    }
  }

  PooledCurlHandle* ret = m_handleStack.top();
  assertx(ret);
  m_handleStack.pop();
  return ret;
}

void CurlHandlePool::store(PooledCurlHandle* handle) {
  Lock lock(m_mutex);
  handle->resetHandle();
  m_handleStack.push(handle);
  pthread_cond_signal(&m_cond);
}

std::map<std::string, CurlHandlePool*> CurlHandlePool::namedPools;

/////////////////////////////////////////////////////////////////////////////
}
