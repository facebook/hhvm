<?hh

namespace HH\SimpleLock {

/*
 * Acquire a cross-request mutual exclusion lock with a unique string name.
 *
 * If the same lock is acquired more than once in a request subsequent attempts
 * to acquire the lock will block until the it is unlocked. The returned
 * Awaitable will resolve when the lock has been acquired.
 *
 * @param string $name - the name of the lock
 */
<<__Native>>
function lock(string $name): Awaitable<void>;

/*
 * Release a cross-request mutual exclusion lock with a unique name.
 *
 * Immediately release a held lock acquired using HH\SimpleLock\lock(), if the
 * lock was never acquired throws an InvalidOperationException.
 *
 * @param string $name - the name of the lock
 */
<<__Native>>
function unlock(string $name): void;

/*
 * Attempt to acquired a cross-request mutual exclusion lock with a unique name.
 *
 * If the lock is unheld it is immediately acquired and true is returned,
 * otherwise we return false and no lock is acquired.
 *
 * @param string $name - the name of the lock
 * @return bool - whether the lock was acquired
 */
<<__Native>>
function try_lock(string $name): bool;

/*
 * Checks if any thread (including the current request) is holding a lock with a
 * unique name.
 *
 * @param string $name - the name of the lock
 * @return bool - whether someone is holding the lock
 */
<<__Native>>
function is_held(string $name): bool;

/*
 * Acquired a cross-request mutual exclusion lock with a unique name and a
 * timeout.
 *
 * Attempts to acquire a lock within a fixed amount of microseconds, if the
 * timeout is reached without acquiring the lock, throws a RuntimeException.
 *
 * @param string $name - the name of the lock
 * @param int $timeout - the number of microseconds to wait for
 */
async function lock_with_timeout(string $name, int $timeout): Awaitable<void> {
  $lwh = lock($name);

  if ($lwh->isFinished()) {
    await $lwh;
    return;
  }

  $swh = \HH\SleepWaitHandle::create($timeout);
  concurrent {
    await async {
      try {
        await $swh;
        \HH\Asio\cancel(
          $lwh,
          new \RuntimeException("Timed out waiting for lock $name"),
        );
      } catch (\Exception $_) {}
    };
    await async {
      await $lwh;
      \HH\Asio\cancel($swh, new \Exception());
    };
  }
}

}
