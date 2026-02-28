/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#pragma once

#include "hphp/util/assertions.h"
#include "hphp/util/compilation-flags.h"

#include "hphp/runtime/vm/jit/types.h"

#include <pthread.h>

namespace HPHP {

struct Func;

namespace jit {

/*
 * Set whether or not the current thread is allowed to acquire the global write
 * lease or a concurrent (Func-specific) write lease.  Used to disable jit for
 * certain requests.
 */
void setMayAcquireLease(bool f);

bool couldAcquireOptimizeLease(const Func*);

/*
 * Used to track which locks are required or acquired in LeaseHolder.
 */
enum class LockLevel {
  /* No locks */
  None,
  /* Func-specific lock only: full concurrent jitting */
  Func,
  /* Kind-specific lock: allows 1 Optimize concurrent with 1 Live translation */
  Kind,
  /* Global lease required: 1 translation at a time */
  Global,
};

struct LeaseHolder {
  LeaseHolder(const Func* f, TransKind kind, bool isWorker = false);
  ~LeaseHolder();

  LeaseHolder(const LeaseHolder&) = delete;
  LeaseHolder& operator=(const LeaseHolder&) = delete;

  /*
   * Returns true iff all the necessary locks were acquired and it's ok to
   * continue with translation.
   */
  explicit operator bool() const {
    return m_level != LockLevel::None;
  }

  /*
   * Check if the combination of Cfg::Jit::Concurrently and kind
   * require acquiring more locks than this LeaseHolder currently has. If so
   * and they can't be acquired, return false. Otherwise, return true.
   */
  bool checkKind(TransKind kind);

 private:
  bool acquireKind(TransKind kind);
  void dropLocks();

  const Func* m_func;

  /*
   * Flags indicating which specific locks were acquired by this object and
   * should be released in its destructor.
   */
  bool m_acquiredGlobal{false};
  bool m_acquiredFunc{false};
  bool m_acquiredThread{false};
  TransKind m_acquiredKind{TransKind::Invalid};
  LockLevel m_level{LockLevel::None};
};

}} // HPHP::jit
