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

#ifndef incl_HPHP_POOL_H_
#define incl_HPHP_POOL_H_

#include "hphp/util/base.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Simple class for object pooling between different threads.
 */
template<typename T>
class Pool : public Synchronizable {
public:
  void push(std::shared_ptr<T> obj) {
    Lock lock(this);
    m_objs.push_back(obj);
    notify();
  }

  std::shared_ptr<T> pop() {
    Lock lock(this);
    while (m_objs.empty()) {
      wait();
    }

    std::shared_ptr<T> obj = m_objs.front();
    m_objs.pop_front();
    return obj;
  }

private:
  std::deque<std::shared_ptr<T> > m_objs;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_POOL_H_
