#ifndef incl_HPHP_CURL_POOL_H
#define incl_HPHP_CURL_POOL_H

#include "hphp/util/mutex.h"

#include <curl/curl.h>

#include <map>
#include <stack>
#include <string>

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////
// Pooled Curl Handle

/**
 *  * This is a helper class used to wrap Curl handles that are pooled.
 *   * Operations on this class are _NOT_ thread safe!
 *    */
struct PooledCurlHandle {
  explicit PooledCurlHandle(int connRecycleAfter)
  : m_handle(nullptr), m_numUsages(0), m_connRecycleAfter(connRecycleAfter) { }
  ~PooledCurlHandle();

  CURL* useHandle();
  void resetHandle();

 private:
  CURL* m_handle;
  int m_numUsages;
  int m_connRecycleAfter;
};

/////////////////////////////////////////////////////////////////////////////
// CurlHandlePool

/* This is a helper class used to implement a process-wide pool of libcurl
 * handles. This provides very large performance benefits, as libcurl handles
 * hold connections open and cache SSL session ids over their lifetimes.
 * All operations on this class are thread safe.
 */
struct CurlHandlePool {
  explicit CurlHandlePool(int poolSize, int waitTimeout, int numConnReuses);
  ~CurlHandlePool();

  PooledCurlHandle* fetch();
  void store(PooledCurlHandle* handle);

  static std::map<std::string, CurlHandlePool*> namedPools;

private:
  std::stack<PooledCurlHandle*> m_handleStack;
  Mutex m_mutex;
  pthread_cond_t m_cond;
  int m_waitTimeout;
};

/////////////////////////////////////////////////////////////////////////////
}
#endif
