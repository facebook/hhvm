/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

// a trick to allow us to parenthesize the type, so the macros
// don't get confused by the commas
namespace detail {
template<typename T> struct get_first_param;
template<typename P> struct get_first_param<void(P)> {
  typedef P type;
};
}

#define ZEND_REQUEST_LOCAL_VECTOR(T, F, N, Z)                   \
  IMPLEMENT_STATIC_REQUEST_LOCAL(                               \
    HPHP::detail::get_first_param<                              \
    void(HPHP::ZendRequestLocalVector<T, F>)>::type, N);        \
  typedef HPHP::ZendRequestLocalVector<T, F>::container Z

template <class T, class F>
struct ZendRequestLocalVector final : RequestEventHandler {
  static_assert(std::is_pointer<T>::value,
                "ZendRequestLocalVector only stores pointers");
  using container = std::vector<T>;
  container& get() { return m_container; }
  void requestInit() override { clear(); }
  void requestShutdown() override { clear(); }
  void vscan(IMarker& mark) const override {
    for (const auto& p : m_container) {
      if (p) p->scan(mark);
    }
  }
private:
  void clear() {
    for (size_t i = 0; i < m_container.size(); ++i) {
      if (m_container[i]) {
        auto element = m_container[i];
        m_container[i] = nullptr;
        m_destroy_callback(element);
      }
    }
    m_container.clear();
    m_container.push_back(nullptr); // don't give out id 0
  }

  container m_container;
  F m_destroy_callback;
};

#define ZEND_REQUEST_LOCAL_MAP(K, V, N)                 \
  IMPLEMENT_STATIC_REQUEST_LOCAL(                       \
    HPHP::detail::get_first_param<                      \
    void(HPHP::ZendRequestLocalMap<K, V>)>::type, N)

template <class K, class V>
struct ZendRequestLocalMap final : RequestEventHandler {
  using container = std::unordered_map<K, V>;
  container& get() { return m_map; }
  void requestInit() override { m_map.clear(); }
  void requestShutdown() override { m_map.clear(); }
  void vscan(IMarker& mark) const override {
    // TODO: t7925927 this is wrong when container has pointers. The below is
    // sufficient for the current usage of this class.
    for (const auto& pair : m_map) {
      mark(&pair.second, sizeof(pair.second));
    }
  }
private:
  container m_map;
};

}

#endif // incl_HPHP_ZEND_REQUEST_LOCAL
