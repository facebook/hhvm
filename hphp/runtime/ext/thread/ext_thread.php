<?hh

/**
 * Gets current thread's ID.
 *
 * @return int - The pthread_self() return.
 *
 */
<<__Native>>
function hphp_get_thread_id(): int;

/**
 * Gets the kernel thread id of the current running thread.
 *
 * @return int - The tid of the current running thread. In Linux, this is
 *   gettid()
 *
 */
<<__Native>>
function hphp_gettid(): int;
