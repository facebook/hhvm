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

#ifndef __HPHP_THREAD_INIT_FINI_H__
#define __HPHP_THREAD_INIT_FINI_H__

#include <runtime/base/hphp_system.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void init_thread_locals(void *arg = NULL) ATTRIBUTE_COLD 
  __attribute__((noinline));
void finish_thread_locals(void *arg = NULL) ATTRIBUTE_COLD
  __attribute__((noinline));

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_THREAD_INIT_FINI_H__
