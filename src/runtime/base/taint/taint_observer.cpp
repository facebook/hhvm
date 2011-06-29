/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifdef TAINTED

#include <runtime/base/taint/taint_observer.h>

namespace HPHP {

IMPLEMENT_THREAD_LOCAL(TaintObserver*, TaintObserver::instance);

void TaintObserver::RegisterAccessed(TaintData const& td) {
  if (!*instance) {
    return;
  }

  // Prevent recursive calls into the TaintObserver. This should never
  // actually happen, except when adding debugging code.

  TaintObserver *tc = *instance;
  *instance = NULL;

  tc->m_current_taint.setTaint(td.getTaint());

  *instance = tc;
}

void TaintObserver::RegisterMutated(TaintData& td) {
  if (!*instance) {
    return;
  }

  // Prevent recursive calls into the TaintObserver.
  TaintObserver *tc = *instance;
  *instance = NULL;

  bitstring t = tc->m_current_taint.getTaint();
  td.setTaint(tc->m_set_mask | (~tc->m_clear_mask & t));

  *instance = tc;
}

}

#endif // TAINTED
