/* Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef MY_RCU_LOCK_INCLUDED
#define MY_RCU_LOCK_INCLUDED

#include <atomic>
#include <functional>

/**
  A class that implements a limited version of the Read-Copy-Update lock pattern

  When you have a global variable that is mostly read and seldomly changed you
  need to make sure your readers get minimal overhead eventually at the cost
  of slowing down your writers.

  It is implemented as an atomic pointer to the protected global (the template
  class). You can safely read from it and you can safely write a new version of
  the global. The problem is how and when to dispose of the old version(s) of
  the global that are swapped out by the write.

  The full RCU implemetnation solves this in a very generic way.
  But we here take a simplification: we assume that there will be frequent times
  when there's not gonna be active readers.

  If we count atomically the active readers (by calling @ref
  MyRcuLock::rcu_read() before reading and by calling @ref
  MyRcuLock::rcu_end_read() when we're done using the value of the global we
  have received from @ref MyRcuLock::rcu_read() we can detect these times by
  simply looping over the atomic count (@ref MyRcuLock::rcu_readers_) and
  waiting for a 0 to come out (this is exactly what
  @ref MyRcuLock::wait_for_no_readers() does). Once we get a zero we know
  that no existing readers will use the old value(s) of the global and new
  readers will get the new value of the global. So if we get that zero we know
  that we can safely dispose of all the old values we hold.

  Intended usage

  We instantiate @ref MyRcuLock once for each global we want to protect.

  To read we instantiate the utility class @ref MyRcuLock::ReadLock and
  we access the value by casting it to the type we need. Once the @ref
  MyRcuLock::ReadLock goes out of scope we can't access the global value
  or copies of it anymore.

  To write we fully prepare the new value of the global and then call
  @ref MyRcuLock::rcu_write(). That returns the old value of the global.

  Now we need to dispose of that old value. For that we wait until there's no
  active readers by calling @ref MyRcuLock::wait_for_no_readers() and
  then, if that succeeds we delete the old value. This is exactly what @ref
  MyRcuLock::write_wait_and_delete() does.

  @tparam T The class of the global protected pointer
*/
template <typename T>
class MyRcuLock {
 public:
  /**
    Set up the RCU lock and the global.

    Initializes the global and the reader counter.

    @param init A value to store into the global. Can be NULL. Should not be
    used after the call
  */
  MyRcuLock(const T *init) {
    rcu_global_.store(init);
    rcu_readers_.store(0);
  }

  /** disabled */
  MyRcuLock(const MyRcuLock &) = delete;
  /** disabled */
  void operator=(const MyRcuLock &) = delete;

  /**
    Destructor.
    Calls @ref MyRcuLock::write_wait_and_delete to set the global
    to NULL and dispose of the old value
  */
  ~MyRcuLock() { write_wait_and_delete(nullptr); }

  /**
    High level read API for readers

    A convenience scope guard class for readers.Use this for all of the RCU
    global readers.
  */
  class ReadLock {
   public:
    /** get the value through the scope guard */
    operator const T *() { return _lock->rcu_global_; }
    /** construct a new read lock scope guard */
    ReadLock(MyRcuLock *l) : _lock(l) { _lock->rcu_read(); }
    ReadLock(const ReadLock &) = delete;
    ~ReadLock() { _lock->rcu_end_read(); }

   protected:
    MyRcuLock operator=(const ReadLock) = delete;
    MyRcuLock *_lock;
  };

  /**
    Low level API: start reading

    Returns a copy of the global that you can safely use until @ref
    MyRcuLock::rcu_end_read() is called
    @warning Don't forget to call @ref MyRcuLock::rcu_end_read() when done.
    @return the copy of the global
  */
  const T *rcu_read() {
    rcu_readers_.fetch_add(1, std::memory_order_relaxed);
    return rcu_global_.load(std::memory_order_relaxed);
  }

  /**
    Low level API: end reading

    Marks the place where the pointer returned by @ref MyRcuLock::read() is
    not going to be accessed any longer.

    The recommended high level API is @ref MyRcuLock::ReadLock
    @warning Each call to @ref MyRcuLock::rcu_read() must be coupled with a
    call to
    @ref MyRcuLock::rcu_end_read(). And the value returned by @ref
    MyRcuLock::read() should not be used after calling @ref
    MyRcuLock::rcu_end_read()
  */
  void rcu_end_read() { rcu_readers_.fetch_sub(1, std::memory_order_relaxed); }

  /**
    Low level API: write a new global and return the old one

    The high level API for writers is @ref MyRcuLock::write_wait_and_delete()

    @note: you need to safely dispose of the returned old global.
    @param newT a pointer to a fully prepared new global that starts getting
    used immediately after being set.
    @return the old value of the global

    @sa @ref MyRcuLock::wait_for_no_readers()
  */
  const T *rcu_write(const T *newT) {
    return rcu_global_.exchange(newT, std::memory_order_release);
  }

  /**
    Low level API: wait for no active readers

    Call this to wait for a state when there's no active readers.
    Optionally pass a functor to call at each check and stop if instructed.

    The high level API to call is @ref MyRcuLock::write_wait_and_delete()

    @retval true if the function was stopped by the functor
    @retval false if an actual moment of 0 readers was detected
  */
  bool wait_for_no_readers() {
    bool stopped = false;
    while (rcu_readers_.load(std::memory_order_relaxed) > 0)
      ;
    return stopped;
  }

  /**
    A RCU writer API

    Stores a new value into the global, waits for when it's safe to dispose of
    the old one and calls delete on it.

    @param newT The new value to store into the global

    @retval true the function was stopped by the functor. The old value was
    lost.
    @retval false operation succeeded and the old value was disposed of
    */
  bool write_wait_and_delete(const T *newT) {
    const T *oldT = this->rcu_write(newT);
    if (!wait_for_no_readers()) {
      delete oldT;
      return false;
    }
    // we leak the oldT here
    return true;
  }

 protected:
  /** the global pointer to protect */
  std::atomic<const T *> rcu_global_;
  /** padding to break the CPU cache lines */
  char rcu_padding_[128];
  /** the readers counter */
  std::atomic<long> rcu_readers_;
};
#endif /* ifndef MY_RCU_LOCK_INCLUDED */
