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
