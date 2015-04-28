<?hh

namespace HH {

/**
 * Get index of the current scheduler context, or 0 if there is none.
 */
<<__Native>>
function asio_get_current_context_idx(): int;

/**
 * Get currently running wait handle in a context specified by its index.
 */
<<__Native>>
function asio_get_running_in_context(int $ctx_idx): ResumableWaitHandle;

/**
 * Get currently running wait handle, or null if there is none.
 */
<<__Native>>
function asio_get_running(): ResumableWaitHandle;

} // namespace

namespace HH\Asio {

/**
 * Wait for a given Awaitable to finish and return its result.
 *
 * Launches a new instance of scheduler to drive asynchronous execution
 * until the provided Awaitable is finished.
 */
function join<T>(Awaitable<T> $awaitable): T {
  invariant(
    $awaitable instanceof WaitHandle,
    'unsupported user-land Awaitable',
  );
  return $awaitable->join();
}

/**
 * Get result of an already finished Awaitable.
 *
 * Throws an InvalidOperationException if the Awaitable is not finished.
 */
function result<T>(Awaitable<T> $awaitable): T {
  invariant(
    $awaitable instanceof WaitHandle,
    'unsupported user-land Awaitable',
  );
  return $awaitable->result();
}

/**
 * Check whether the given Awaitable has finished.
 */
function has_finished<T>(Awaitable<T> $awaitable): bool {
  invariant(
    $awaitable instanceof WaitHandle,
    'unsupported user-land Awaitable',
  );
  return $awaitable->isFinished();
}

} // namespace
