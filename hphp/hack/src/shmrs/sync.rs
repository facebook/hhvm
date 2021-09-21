// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::UnsafeCell;
use std::ops::{Deref, DerefMut};

use crate::error::Errno;

/// A reader-writer lock that can be used in a shared-memory
/// (inter-process) context.
///
/// This struct is the moveable and unusable as long as you haven't
/// called `initialize` or `attach` on it.
///
/// You should call `initialize` or `attach` to consider the memory
/// location as pinned from which point you can acquire the lock.
pub struct RwLock<T> {
    lock: libc::pthread_rwlock_t,
    data: UnsafeCell<T>,
}

/// A reference to an initialized or attached reader-writer lock.
///
/// After calling `initialize` or `attach` on `RwLock`, I'd like
/// the interface to be as safe as possible (to avoid users shooting)
/// themselves in the foot. Therefore, I provide this wrapper type
/// around an immutable reference to `RwLock`.
///
/// Having this wrapper type ensures the underlying `RwLock` isn't
/// modified while a reference to it is live.
///
/// A side-effect of having two types (`RwLock` and `RwLockRef`) is
/// that it is impossible for a process to acquire a lock without
/// calling either `initialize` or `attach`.
pub struct RwLockRef<'a, T>(&'a RwLock<T>);

impl<T> RwLock<T> {
    /// Create a new uninitialized lock.
    ///
    /// Note that the lock can still be moved around freely. However, you
    /// can't actually acquire it yet. Initialize or attach the lock by
    /// calling `initialize` or `acquire`.
    ///
    /// We are not using a RAII API here, because we can't guarantee that the
    /// initializing process outlives other attaching processes. Destroying
    /// a lock that's still in use by other processes is highly unsafe.
    ///
    /// We use a two-step initialization procedure because we need two types:
    /// One type to hold the actual data and one type that holds a reference
    /// to that data, and as such avoids accidentally moving or modifying the
    /// data.
    pub fn new(value: T) -> Self {
        Self {
            lock: libc::PTHREAD_RWLOCK_INITIALIZER,
            data: UnsafeCell::new(value),
        }
    }

    /// Initialize the lock and return a reference to it.
    ///
    /// The reference can be used to acquire the actual lock. Using a
    /// reference prevents the underlying data from being moved while
    /// the lock is in use.
    ///
    /// Safety:
    ///  - No other process should try to initialize the lock at the same.
    ///  - The lock should not be held by another process.
    ///  - As long as the lock is in use, you can't initialize it again!
    ///  - You shouldn't be mutating the `RwLock` after `initialize`
    ///    or attached are called.
    pub unsafe fn initialize(&self) -> Result<RwLockRef<'_, T>, Errno> {
        let mut attr: libc::pthread_rwlockattr_t = std::mem::zeroed();

        Errno::from(libc::pthread_rwlockattr_init(&mut attr as *mut _))?;

        Self::set_prefer_writer(&mut attr as *mut _)?;

        // Allow access from multiple processes
        Errno::from(libc::pthread_rwlockattr_setpshared(
            &mut attr as *mut _,
            libc::PTHREAD_PROCESS_SHARED,
        ))?;
        Errno::from(libc::pthread_rwlock_init(
            self.lock_ptr(),
            &attr as *const _,
        ))?;

        Ok(self.attach())
    }

    /// Attach to an already initialized lock.
    ///
    /// Safety:
    ///  - The lock should already be initialized by another process
    ///    (or the calling process)
    pub unsafe fn attach(&self) -> RwLockRef<'_, T> {
        RwLockRef(self)
    }

    #[inline(always)]
    fn lock_ptr(&self) -> *mut libc::pthread_rwlock_t {
        &self.lock as *const _ as *mut _
    }

    #[cfg(target_os = "linux")]
    unsafe fn set_prefer_writer(attr: *mut libc::pthread_rwlockattr_t) -> Result<(), Errno> {
        // Not defined in the libc crate. Linux specific. See pthread.h.
        const LIBC_PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP: libc::c_int = 2;
        Errno::from(libc::pthread_rwlockattr_setkind_np(
            attr,
            LIBC_PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP,
        ))
    }

    #[cfg(not(target_os = "linux"))]
    unsafe fn set_prefer_writer(_attr: *mut libc::pthread_rwlockattr_t) -> Result<(), Errno> {
        // Non-Linux Oses don't have this flag.
        Ok(())
    }

    /// Destroy the underlying lock.
    ///
    /// Safety:
    ///   - Subsequent use of the lock is undefined until the lock is
    ///     reinitialized by calling `initialize`.
    ///   - No thread must hold the lock.
    ///   - Attempting to destroy an uninitialized lock is undefined behavior.
    pub unsafe fn destroy(&mut self) -> Result<(), Errno> {
        Errno::from(libc::pthread_rwlock_destroy(self.lock_ptr()))
    }
}

impl<'a, T> RwLockRef<'a, T> {
    /// Locks this rwlock with shared read access, blocking the current
    /// thread until it can be acquired.
    ///
    /// Note that, according to the pthread_rwlock_rdlock manual page, this
    /// does support recursive locking.
    pub fn read(self) -> Result<RwLockReadGuard<'a, T>, Errno> {
        // Safety: A RwLockRef can only be obtained by calling
        // RwLock::initialize or -acquire. Therefore, we should
        // have a pointer to a valid rwlock.
        let success = Self::try_fastlock(|| unsafe {
            libc::pthread_rwlock_tryrdlock(self.0.lock_ptr()) == 0
        });
        if !success {
            unsafe {
                Errno::from(libc::pthread_rwlock_rdlock(self.0.lock_ptr()))?;
            }
        }
        Ok(RwLockReadGuard { lock: self })
    }

    /// Locks this rwlock with exclusive write access, blocking the current
    /// thread until it can be acquired.
    ///
    /// Note that, according to the pthread_rwlock_wrlock manual page, this
    /// returns EDEADLK of the current thread already holds the lock.
    pub fn write(self) -> Result<RwLockWriteGuard<'a, T>, Errno> {
        // Safety: A RwLockRef can only be obtained by calling
        // RwLock::initialize or -acquire. Therefore, we should
        // have a pointer to a valid rwlock.
        let success = Self::try_fastlock(|| unsafe {
            libc::pthread_rwlock_trywrlock(self.0.lock_ptr()) == 0
        });
        if !success {
            unsafe {
                Errno::from(libc::pthread_rwlock_wrlock(self.0.lock_ptr()))?;
            }
        }
        Ok(RwLockWriteGuard { lock: self })
    }

    /// Unlocks the rwlock.
    ///
    /// Safety: The thread must be locked, duh.
    unsafe fn unlock(self) {
        Errno::from(libc::pthread_rwlock_unlock(self.0.lock_ptr())).unwrap();
    }

    #[inline]
    fn try_fastlock(try_lock: impl Fn() -> bool) -> bool {
        for counter in 1..=20 {
            if try_lock() {
                return true;
            }

            if counter <= 10 {
                for _ in 0..(4 << counter) {
                    std::hint::spin_loop();
                }
            } else {
                std::thread::yield_now();
            }
        }

        false
    }
}

impl<'a, T> Clone for RwLockRef<'a, T> {
    fn clone(&self) -> Self {
        Self(self.0)
    }
}

impl<'a, T> Copy for RwLockRef<'a, T> {}

/// Read guard for `RwLock`
pub struct RwLockReadGuard<'a, T> {
    lock: RwLockRef<'a, T>,
}

impl<T> Deref for RwLockReadGuard<'_, T> {
    type Target = T;

    fn deref(&self) -> &T {
        // Safety: lock semantics!
        unsafe { &*self.lock.0.data.get() }
    }
}

impl<T> Drop for RwLockReadGuard<'_, T> {
    fn drop(&mut self) {
        // Safety: we have a locked lock!
        unsafe {
            self.lock.unlock();
        }
    }
}

/// Write guard for `RwLock`
pub struct RwLockWriteGuard<'a, T> {
    lock: RwLockRef<'a, T>,
}

impl<T> Deref for RwLockWriteGuard<'_, T> {
    type Target = T;

    fn deref(&self) -> &T {
        // Safety: lock semantics!
        unsafe { &*self.lock.0.data.get() }
    }
}

impl<T> DerefMut for RwLockWriteGuard<'_, T> {
    fn deref_mut(&mut self) -> &mut T {
        // Safety: lock semantics!
        unsafe { &mut *self.lock.0.data.get() }
    }
}

impl<T> Drop for RwLockWriteGuard<'_, T> {
    fn drop(&mut self) {
        // Safety: we have a locked lock!
        unsafe {
            self.lock.unlock();
        }
    }
}

unsafe impl<T: Send> Send for RwLock<T> {}
unsafe impl<T: Send + Sync> Sync for RwLock<T> {}
unsafe impl<T: Sync> Sync for RwLockReadGuard<'_, T> {}
unsafe impl<T: Sync> Sync for RwLockWriteGuard<'_, T> {}

#[cfg(test)]
mod integration_tests {
    use super::*;

    use std::mem::MaybeUninit;
    use std::time::Duration;

    use nix::sys::wait::WaitStatus;
    use nix::unistd::ForkResult;
    use rand::prelude::*;

    struct Incr {
        counter: RwLock<u64>,
    }

    #[test]
    fn test_incrementor() {
        // Test scenario: Launch 20 processes that each increment a counter
        // 1000 times. Each iteration has a probability of 0.5 to increase
        // the counter by 1 and sleep a bit while holding the lock. Or do a
        // read-sleep cycle 3 times while holding the readers lock, assuring
        // the value doesn't change.
        let mmap_size = std::mem::size_of::<Incr>();
        let mmap_ptr = unsafe {
            libc::mmap(
                std::ptr::null_mut(),
                mmap_size,
                libc::PROT_READ | libc::PROT_WRITE,
                libc::MAP_SHARED | libc::MAP_ANONYMOUS,
                -1,
                0,
            )
        };
        assert_ne!(mmap_ptr, libc::MAP_FAILED);
        let incr_ptr: *mut MaybeUninit<Incr> = mmap_ptr as *mut _;
        let incr: &'static mut MaybeUninit<Incr> =
            // Safety:
            //  - Pointer is not null
            //  - Pointer is aligned on a page
            //  - This is the only reference to the data, and the lifetime is
            //    static as we don't unmap the memory.
            unsafe { &mut *incr_ptr };
        // Safety: Initialize the memory properly
        let incr = unsafe {
            incr.as_mut_ptr().write(Incr {
                counter: RwLock::new(0),
            });
            incr.assume_init_mut()
        };
        // Safety: We are the only ones to attach to this lock!
        let lock = unsafe { incr.counter.initialize().unwrap() };

        const NUM_PROCS: u64 = 10;
        const NUM_INCRS_PER_PROC: u64 = 100;
        const NUM_CONSEQ_READS: u64 = 2;
        const OP_SLEEP: Duration = Duration::from_millis(1);

        let mut child_procs = vec![];
        for child_index in 0..NUM_PROCS {
            match unsafe { nix::unistd::fork() }.unwrap() {
                ForkResult::Parent { child } => {
                    child_procs.push(child);
                }
                ForkResult::Child => {
                    let mut seed: [u8; 32] = [0; 32];
                    seed[0..8].copy_from_slice(&child_index.to_be_bytes());
                    let mut rng = StdRng::from_seed(seed);

                    let mut num_incrs = 0;
                    while num_incrs < NUM_INCRS_PER_PROC {
                        if rng.gen_bool(0.5) {
                            let mut guard = lock.write().unwrap();
                            *guard += 1;
                            std::thread::sleep(OP_SLEEP);
                            drop(guard);
                            num_incrs += 1;
                        } else {
                            let guard = lock.read().unwrap();
                            let init_val = *guard;
                            for _ in 0..NUM_CONSEQ_READS {
                                std::thread::sleep(OP_SLEEP);
                                assert_eq!(*guard, init_val);
                            }
                            drop(guard);
                        }
                    }

                    std::process::exit(0)
                }
            }
        }

        for pid in child_procs {
            match nix::sys::wait::waitpid(pid, None).unwrap() {
                WaitStatus::Exited(_, status) => assert_eq!(status, 0),
                status => panic!("unexpected status for pid {:?}: {:?}", pid, status),
            }
        }

        assert_eq!(*lock.read().unwrap(), NUM_PROCS * NUM_INCRS_PER_PROC);

        assert_eq!(unsafe { libc::munmap(mmap_ptr, mmap_size) }, 0);
    }
}
