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
#ifndef incl_HPHP_WRITELEASE_H_
#define incl_HPHP_WRITELEASE_H_

#include "hphp/util/assertions.h"
#include "hphp/util/compilation-flags.h"
#include "hphp/runtime/base/runtime-option.h"

#include "hphp/runtime/vm/jit/types.h"

#include <pthread.h>

namespace HPHP {

struct Func;

namespace jit {

/*
 * The write Lease guards write access to the translation caches, srcDB, and
 * TransDB. The term "lease" is meant to indicate that the right of ownership
 * is conferred for a long, variable time: often the entire length of a
 * request. If a request is not actively translating, it will perform a "hinted
 * drop" of the lease: the lease is unlocked but all calls to acquire(false)
 * from other threads will fail for a short period of time.
 */

struct LeaseHolderBase;
struct Lease {
  friend struct LeaseHolder;

  Lease();
  ~Lease();

  /*
   * Returns true iff the Lease is locked and the current thread is the owner.
   */
  bool amOwner() const;

  int64_t hintKept()    const { return m_hintKept; }
  int64_t hintGrabbed() const { return m_hintGrabbed; }

  /*
   * Returns true iff the current thread would be able to acquire the lease now
   * (or already owns it).
   */
  bool couldBeOwner() const;

  static void mayLock(bool f);
  static void mayLockConcurrent(bool f);

private:
  // acquire: also returns true if we are already the writer.
  bool acquire(bool blocking = false);
  void drop(int64_t hintExpireDelay = 0);
  void gremlinUnlockImpl();

  // Since there's no portable, universally invalid pthread_t, explicitly
  // represent the held <-> unheld state machine with m_held.
  pthread_t         m_owner;
  std::atomic<bool> m_held{false};
  pthread_mutex_t   m_lock;

  // Timestamp for when a hinted lease drop (see block comment above) will
  // expire.
  int64_t m_hintExpire{0};

  // Statistics about how many times the write lease is picked back up by the
  // thread that did a hinted drop.
  int64_t m_hintKept{0};
  int64_t m_hintGrabbed{0};
};

enum class LeaseAcquire {
  ACQUIRE,
  BLOCKING
};

struct LeaseHolder {
  LeaseHolder(Lease& l, const Func* f, TransKind kind);
  ~LeaseHolder();

  /*
   * Returns true iff all the necessary locks were acquired and it's ok to
   * continue with translation.
   */
  explicit operator bool() const {
    return m_canTranslate;
  }

  /*
   * If the combination of RuntimeOption::EvalJitConcurrently and kind require
   * holding the global write lease and it can't be acquired, return false.
   */
  bool checkKind(TransKind kind);

  /*
   * Returns whether the combination of RuntimeOption::EvalJitConcurrently and
   * kind require holding the global write lease.
   */
  static bool NeedGlobal(TransKind kind);

 private:
  void dropLocks();

  Lease& m_lease;
  const Func* m_func;

  /*
   * Did this object acquire all the locks it was supposed to in its
   * constructor?
   */
  bool m_canTranslate{false};

  /*
   * Flags indicating which specific locks were acquired by this object and
   * should be released in its destructor.
   */
  bool m_acquired{false};
  bool m_acquiredFunc{false};
  bool m_acquiredThread{false};
};

Lease& GetWriteLease();

}} // HPHP::jit

#endif /* incl_HPHP_WRITELEASE_H_ */
