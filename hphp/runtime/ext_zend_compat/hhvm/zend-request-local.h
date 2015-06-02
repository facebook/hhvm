/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_ZEND_REQUEST_LOCAL
#define incl_HPHP_ZEND_REQUEST_LOCAL

#include "hphp/runtime/base/request-local.h"
#include <unordered_map>
#include <vector>
#include "hphp/runtime/base/request-event-handler.h"

namespace HPHP {

#define ZEND_REQUEST_LOCAL_VECTOR(T, N) static __thread HPHP::RequestLocal<HPHP::ZendRequestLocalVector<T> > N;
template <class T>
struct ZendRequestLocalVector final : RequestEventHandler {
  using container = std::vector<T>;
  void requestInit() override { clear(); }
  void requestShutdown() override { clear(); }
  container& get() { return m_container; }
private:
  void clear() {
    m_container.clear();
    m_container.push_back(nullptr); // don't give out id 0
  }
private:
  container m_container;
};

#define ZEND_REQUEST_LOCAL_MAP(K, V, N) static __thread HPHP::RequestLocal<HPHP::ZendRequestLocalMap<K,V> > N;
template <class K, class V>
struct ZendRequestLocalMap final : RequestEventHandler {
  typedef std::unordered_map<K, V> container;
  void requestInit() override { m_map.clear(); }
  void requestShutdown() override { m_map.clear(); }
  container& get() { return m_map; }
private:
  container m_map;
};

}

#endif // incl_HPHP_ZEND_REQUEST_LOCAL
