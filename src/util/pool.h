/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_POOL_H__
#define __HPHP_POOL_H__

#include <util/base.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Simple class for object pooling between different threads.
 */
template<typename T>
class Pool : public Synchronizable {
public:
  void push(boost::shared_ptr<T> obj) {
    Lock lock(this);
    m_objs.push_back(obj);
    notify();
  }

  boost::shared_ptr<T> pop() {
    Lock lock(this);
    while (m_objs.empty()) {
      wait();
    }

    boost::shared_ptr<T> obj = m_objs.front();
    m_objs.pop_front();
    return obj;
  }

private:
  std::deque<boost::shared_ptr<T> > m_objs;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_POOL_H__
