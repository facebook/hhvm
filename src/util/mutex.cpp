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

#include "mutex.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Mutex::Mutex(bool reentrant /* = true */) {
  pthread_mutexattr_init(&m_mutexattr);
  if (reentrant) {
    pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_RECURSIVE);
  } else {
    pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_ADAPTIVE_NP);
  }
  pthread_mutex_init(&m_mutex, &m_mutexattr);
}

///////////////////////////////////////////////////////////////////////////////
}
