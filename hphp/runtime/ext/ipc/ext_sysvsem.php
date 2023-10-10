<?hh

/**
 * sem_acquire() blocks (if necessary) until the semaphore can be acquired. A
 *   process attempting to acquire a semaphore which it has already acquired
 *   will block forever if acquiring the semaphore would cause its maximum
 *   number of semaphore to be exceeded.  After processing a request, any
 *   semaphores acquired by the process but not explicitly released will be
 *   released automatically and a warning will be generated.
 *
 * @param resource $sem_identifier - sem_identifier is a semaphore resource,
 *   obtained from sem_get().
 *
 * @param bool $nowait - Specifies if the process shouldn't wait for the
 * semaphore to be acquired. If set to true, the call will return false
 * immediately if a semaphore cannot be immediately acquired.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function sem_acquire(resource $sem_identifier, bool $nowait = false): bool;

/**
 * sem_get() returns an id that can be used to access the System V semaphore
 *   with the given key.  A second call to sem_get() for the same key will
 *   return a different semaphore identifier, but both identifiers access the
 *   same underlying semaphore.
 *
 * @param int $key
 * @param int $max_acquire - The number of processes that can acquire the
 *   semaphore simultaneously is set to max_acquire.
 * @param int $perm - The semaphore permissions. Actually this value is set
 *   only if the process finds it is the only process currently attached to the
 *   semaphore.
 * @param bool $auto_release - Specifies if the semaphore should be
 *   automatically released on request shutdown.
 *
 * @return mixed - Returns a positive semaphore identifier on success, or
 *   FALSE on error.
 *
 */
<<__Native>>
function sem_get(int $key,
                 int $max_acquire = 1,
                 int $perm = 0666,
                 bool $auto_release = true): mixed;

/**
 * sem_release() releases the semaphore if it is currently acquired by the
 *   calling process, otherwise a warning is generated.  After releasing the
 *   semaphore, sem_acquire() may be called to re-acquire it.
 *
 * @param resource $sem_identifier - A Semaphore resource handle as returned
 *   by sem_get().
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function sem_release(resource $sem_identifier): bool;

/**
 * sem_remove() removes the given semaphore.  After removing the semaphore, it
 *   is no more accessible.
 *
 * @param resource $sem_identifier - A semaphore resource identifier as
 *   returned by sem_get().
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function sem_remove(resource $sem_identifier): bool;
