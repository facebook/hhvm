/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_MUTEX_H_
#define incl_HPHP_MUTEX_H_

#include <pthread.h>
#include <time.h>

// This fixes a bug in tbb headers
// make sure these aren't defined on cygwin
#ifdef __CYGWIN__
# ifdef _WIN32
#  undef _WIN32
# endif
# ifdef _WIN64
#  undef _WIN64
# endif
#endif

#include <tbb/concurrent_hash_map.h>

#include "hphp/util/portability.h"
#include "hphp/util/assertions.h"
#include "hphp/util/rank.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

template <bool enableAssertions>
class BaseMutex {
private:
#ifdef DEBUG
  static const int kMagic = 0xba5eba11;
  int          m_magic;
  Rank         m_rank;
  // m_owner/m_hasOwner for keeping track of lock ownership, useful for debugging
  pthread_t    m_owner;
  unsigned int m_acquires;
  bool         m_recursive;
  bool         m_hasOwner;
#endif
  inline void recordAcquisition() {
#ifdef DEBUG
    if (enableAssertions) {
      assert(!m_hasOwner ||
             pthread_equal(m_owner, pthread_self()));
      assert(m_acquires == 0 ||
             pthread_equal(m_owner, pthread_self()));
      pushRank(m_rank);
      m_hasOwner = true;
      m_owner    = pthread_self();
      m_acquires++;
      assert(m_recursive || m_acquires == 1);
    }
#endif
  }
  inline void invalidateOwner() {
#ifdef DEBUG
    if (enableAssertions) {
      m_hasOwner = false;
      m_acquires = 0;
    }
#endif
  }
  inline void recordRelease() {
#ifdef DEBUG
    if (enableAssertions) {
      popRank(m_rank);
      assertOwnedBySelf();
      assert(m_acquires > 0);
      if (--m_acquires == 0) {
        m_hasOwner = false;
      }
    }
#endif
  }
public:
  inline void assertNotOwned() const {
#ifdef DEBUG
    if (enableAssertions) {
      assert(!m_hasOwner);
      assert(m_acquires == 0);
    }
#endif
  }
  inline void assertOwnedBySelf() const {
#ifdef DEBUG
    if (enableAssertions) {
      assert(m_hasOwner);
      assert(pthread_equal(m_owner, pthread_self()));
      assert(m_acquires > 0);
    }
#endif
  }
public:
  explicit BaseMutex(bool recursive = true, Rank r = RankUnranked) {
    pthread_mutexattr_init(&m_mutexattr);
    if (recursive) {
      pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_RECURSIVE);
    } else {
#if (defined(__APPLE__) || defined(__CYGWIN__) || defined(__MINGW__) || \
    defined(_MSC_VER))
      pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_DEFAULT);
#else
      pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_ADAPTIVE_NP);
#endif
    }
    pthread_mutex_init(&m_mutex, &m_mutexattr);
#ifdef DEBUG
    m_rank = r;
    m_magic = kMagic;
    invalidateOwner();
    m_recursive = recursive;
#endif
  }
  ~BaseMutex() {
#ifdef DEBUG
    assert(m_magic == kMagic);
#endif
    assertNotOwned();
    pthread_mutex_destroy(&m_mutex);
    pthread_mutexattr_destroy(&m_mutexattr);
#ifdef DEBUG
    m_magic = ~m_magic;
#endif
  }

  bool tryLock() {
#ifdef DEBUG
    assert(m_magic == kMagic);
#endif
    bool success = !pthread_mutex_trylock(&m_mutex);
    if (success) {
      recordAcquisition();
      assertOwnedBySelf();
    }
    return success;
  }

  void lock() {
#ifdef DEBUG
    assert(m_magic == kMagic);
    checkRank(m_rank);
#endif
    UNUSED int ret = pthread_mutex_lock(&m_mutex);
    assert(ret == 0);

    recordAcquisition();
    assertOwnedBySelf();
  }

  void unlock() {
#ifdef DEBUG
    assert(m_magic == kMagic);
#endif
    recordRelease();
    UNUSED int ret = pthread_mutex_unlock(&m_mutex);
    assert(ret == 0);
  }

private:
  BaseMutex(const BaseMutex &); // suppress
  BaseMutex &operator=(const BaseMutex &); // suppress

protected:
  pthread_mutexattr_t m_mutexattr;
  pthread_mutex_t     m_mutex;
};

/**
 * Standard recursive mutex, which can be used for condition variables.
 * This mutex does not support enabling assertions
 */
class Mutex : public BaseMutex<false> {
public:
  explicit Mutex(bool recursive = true, Rank rank = RankUnranked) :
    BaseMutex<false>(recursive, rank) {}
  explicit Mutex(Rank rank, bool recursive = true) :
    BaseMutex<false>(recursive, rank) {}
  pthread_mutex_t &getRaw() { return m_mutex; }
};

/**
 * Simple recursive mutex, which does not expose the underlying raw
 * pthread_mutex_t. This allows this mutex to support enabling assertions
 */
class SimpleMutex : public BaseMutex<true> {
public:
  explicit SimpleMutex(bool recursive = true, Rank rank = RankUnranked) :
    BaseMutex<true>(recursive, rank) {}
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Read-write lock wrapper.
 */
class ReadWriteMutex {
#ifdef DEBUG
/*
 * We have a track record of self-deadlocking on these, and our pthread
 * implementation tends to do crazy things when a rwlock is double-wlocked,
 * so check and assert early in debug builds.
 */
  static constexpr pthread_t InvalidThread = (pthread_t)0;
  pthread_t m_writeOwner;
  Rank m_rank;
#endif

  void invalidateWriteOwner() {
#ifdef DEBUG
    m_writeOwner = InvalidThread;
#endif
  }

  void recordWriteAcquire() {
#ifdef DEBUG
    assert(m_writeOwner == InvalidThread);
    m_writeOwner = pthread_self();
#endif
  }

  void assertNotWriteOwner() {
#ifdef DEBUG
    assert(m_writeOwner != pthread_self());
#endif
  }

  void assertNotWriteOwned() {
#ifdef DEBUG
    assert(m_writeOwner == InvalidThread);
#endif
  }

public:
  explicit ReadWriteMutex(Rank rank = RankUnranked)
#ifdef DEBUG
    : m_rank(rank)
#endif
  {
    invalidateWriteOwner();
    pthread_rwlock_init(&m_rwlock, nullptr);
  }

  ~ReadWriteMutex() {
    assertNotWriteOwned();
    pthread_rwlock_destroy(&m_rwlock);
  }

  void acquireRead() {
    /*
     * Atomically downgrading a write lock to a read lock is not part of the
     * pthreads standard. See task #528421.
     */
    assertNotWriteOwner();
    pushRank(m_rank);
    pthread_rwlock_rdlock(&m_rwlock);
    /*
     * Again, see task #528421.
     */
    assertNotWriteOwned();
  }

  void acquireWrite() {
    assertNotWriteOwner();
    pushRank(m_rank);
    pthread_rwlock_wrlock(&m_rwlock);
    assertNotWriteOwned();
    recordWriteAcquire();
  }

  bool attemptRead() { return !pthread_rwlock_tryrdlock(&m_rwlock); }
  bool attemptWrite() { return !pthread_rwlock_trywrlock(&m_rwlock); }
  void release() {
#ifdef DEBUG
    popRank(m_rank);
    if (m_writeOwner == pthread_self()) {
      invalidateWriteOwner();
    }
#endif
    pthread_rwlock_unlock(&m_rwlock);
  }

private:
  ReadWriteMutex(const ReadWriteMutex &); // suppress
  ReadWriteMutex &operator=(const ReadWriteMutex &); // suppress

  pthread_rwlock_t m_rwlock;
};

/*
 * A ranked wrapper around tbb::concurrent_hash_map.
 */
template<typename K, typename V, typename H=K, Rank R=RankUnranked>
class RankedCHM : public tbb::concurrent_hash_map<K, V, H> {
  typedef tbb::concurrent_hash_map<K, V, H> RawCHM;
 public:
  class accessor : public RawCHM::accessor {
    bool freed;
   public:
    accessor() : freed(false) { pushRank(R); }
    ~accessor() { if (!freed) popRank(R); }
    void release() {
      RawCHM::accessor::release();
      popRank(R);
      freed = true;
    }
  };
  class const_accessor : public RawCHM::const_accessor {
    bool freed;
   public:
    const_accessor() : freed(false) { pushRank(R); }
    ~const_accessor() { if (!freed) popRank(R); }
    void release() {
      RawCHM::const_accessor::release();
      popRank(R);
      freed = true;
    }
  };

  bool find(const_accessor& a, const K& k) const { return RawCHM::find(a, k); }
  bool find(accessor& a, const K& k)         { return RawCHM::find(a, k); }
  bool insert(accessor& a, const K& k)       { return RawCHM::insert(a, k); }
  bool insert(const_accessor& a, const K& k) { return RawCHM::insert(a, k); }
  bool erase(accessor& a)                    { return RawCHM::erase(a); }
  bool erase(const_accessor& a)              { return RawCHM::erase(a); }
  bool erase(const K& k)                     { return RawCHM::erase(k); }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_MUTEX_H_
