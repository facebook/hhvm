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

#include <util/ssl_init.h>
#include <util/mutex.h>
#include <util/process.h>
#include <openssl/crypto.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static Mutex *locks;

static unsigned long callback_thread_id() {
  return (unsigned long)Process::GetThreadId();
}

static void callback_locking(int mode, int type, const char *file, int line) {
  if (mode & CRYPTO_LOCK) {
    locks[type].lock();
  } else {
    locks[type].unlock();
  }
}

static volatile bool isSSLInit = false;
static Mutex lockSSLInit;

void SSLInit::init() {
  if (!isSSLInit) {
    lockSSLInit.lock();
    if (!isSSLInit) {
      isSSLInit = true;
      locks = new Mutex[CRYPTO_num_locks()];
      CRYPTO_set_id_callback((unsigned long (*)())callback_thread_id);
      CRYPTO_set_locking_callback(
        (void (*)(int mode, int type, const char *file, int line))
        callback_locking);
    }
    lockSSLInit.unlock();
  }
}


///////////////////////////////////////////////////////////////////////////////
}
