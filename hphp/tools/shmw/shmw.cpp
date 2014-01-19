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

#include <stdio.h>
#include <unistd.h>
#include "hphp/util/shm-counter.h"

using namespace HPHP;
///////////////////////////////////////////////////////////////////////////////

int main() {
  for (int i = 0; ; i++) {
    if (ShmCounters::s_shmCounters == nullptr) {
      if (!ShmCounters::initialize(false)) break;
    }
    fprintf(stderr, "%d\n", i);
    ShmCounters::dump();
    fprintf(stderr, "\n");
    sleep(1);
  }
  return 0;
}
