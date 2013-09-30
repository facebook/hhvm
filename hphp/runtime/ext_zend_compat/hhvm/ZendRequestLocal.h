/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#define ZEND_REQUEST_LOCAL_LIST(T, N) static __thread HPHP::RequestLocal<ZendRequestLocalList<T> > N;
template <class T>
class ZendRequestLocalList : public HPHP::RequestEventHandler {
  private:
    void clear() {
      m_list.clear();
      m_list.push_back(nullptr); // don't give out id 0
    }
  public:
    typedef std::vector<T> list;
    virtual void requestInit() {
      clear();
    }
    virtual void requestShutdown() {
      clear();
    }
    list& get() {
      return m_list;
    }
  private:
    list m_list;
};

#define ZEND_REQUEST_LOCAL_MAP(K, V, N) static __thread HPHP::RequestLocal<ZendRequestLocalMap<K,V> > N;
template <class K, class V>
class ZendRequestLocalMap : public HPHP::RequestEventHandler {
  public:
    typedef std::unordered_map<K, V> list;
    virtual void requestInit() {
      m_map.clear();
    }
    virtual void requestShutdown() {
      m_map.clear();
    }
    list& get() {
      return m_map;
    }
  private:
    list m_map;
};

#endif // incl_HPHP_ZEND_REQUEST_LOCAL
