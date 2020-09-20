<?hh // partial

/* A Watchman (https://facebook.github.io/watchman/) API for HHVM.
 *
 * Example usage:
 *
 *  // Simple one-shot query.
 *  print(watchman_run('["clock", "/home/jbower/"]')->join());
 *  // => {"clock":"c:1459617179:3235334:1:388831","version":"4.5.0"}
 *
 *  // Set-up a subscription to a directory
 *  function cbfunc(
 *    string $path,
 *    string $query,
 *    string $name,
 *    string $data,
 *    string $socket_path,
 *  ) : void {
 *    print(implode(", ", array($path, $query, $name, $data, $socket_path)));
 *    // => /tmp, {"fields": ["name"]}, X, {"files", ["foo.txt", "bar.php"],
 *  }
 *
 *  watchman_subscribe('{"fields": ["name"]}', '/tmp', 'X', 'cbfunc')->join();
 *  ...
 *  // Check to see if subscription is active
 *  print(watchman_check_sub('X')); // => true
 *  ...
 *  // Terminate subscription
 *  watchman_unsubscribe('X')->join();
 *  ...
 *  // Confirm subscription is no-longer active
 *  print(watchman_check_sub('X')); // => false
 */

namespace HH {

/* Asynchronously run a one-shot Watchman query. A JSON encoded response string
 * is returned after a successful join. See the Watchman docs for details of
 * queries and responses.
 *
 * Any errors will be reported via an exception from this call or on join.
 */
<<__Native>>
function watchman_run(
  string $json_query,
  ?string $socket_name = null,
): Awaitable<string>;


/* Set-up a new named Watchman subscription with a callback to be invoked for
 * each update.
 *
 * Callbacks is invoked via a local xbox server task and as such are effectively
 * each a new request of their own. Data must be communicated from a callback to
 * other requests/scripts via a mechanism like APC. Callbacks are invoked with
 * string arguments: $path, $json_query, $sub_name, $data, $socket_path.
 *
 * Subscription names must be unique across the entire HHVM server process.
 *
 * A subscription Will be active by the time this asynchronous request is
 * successfully joined.
 *
 * An error in setting up a subscription will be reported via an exception
 * thrown from this function or on result join as appropriate. Errors during
 * callback execution are handled in watchman_check_sub().
 */
<<__Native>>
function watchman_subscribe(
  string $json_query,
  string $path,
  string $sub_name,
  string $callback,
  ?string $socket_name = null,
): Awaitable<void>;


/* Synchronously check the status of a subscription. Possible return values:
 *   true - subscription exists
 *   false - subscription does not exist
 *   exception is thrown - subscription exists but an error has occurred
 *
 * If an exception is thrown it is cleared by running this function, however the
 * subscription may longer be active depending on the error. Future calls to
 * watchman_check_sub() will continue to return true unless another error is
 * raised, or a watchman_unsubscribe() request completes.
 *
 * If multiple errors occur only the first is captured.
 */
<<__Native>>
function watchman_check_sub(string $sub_name): bool;


/* Produce an awaitable which will block until all currently outstanding
 * processing for the named subscription is complete.
 *
 * At the first point there is no more outstanding processing to be done
 * the awaitable will be marked as fulfilled. So, if all processing finishes and
 * the PHP code has not joined yet but then more processing starts, a subsequent
 * join() will still return immediately even more processing is happening. This
 * arises as it's not possible to un-fulfill an awaitable.
 *
 * The returned bool is true on successful sync or false on timeout if set.
 */
<<__Native>>
function watchman_sync_sub(
  string $sub_name,
  int $timeout_ms = 0,
): Awaitable<bool>;


/* Asynchronously terminate a subscription. By the time this operation is
 * joined() the callback from watchman_subscribe() will no longer be active, and
 * will not be invoked again for this subscription. The degree to which pending
 * updates are processed after an unsubscription is requested is unspecified.
 */
<<__Native>>
function watchman_unsubscribe(string $sub_name): Awaitable<string>;

/**
 * This should be bumped with every non-backwards compatible API change.
 * 1 => first version
 */
function ext_watchman_version(): int {
  return (int)\phpversion("watchman");
}

} // namespace HH


namespace __SystemLib {

/*
 * Internal use only.
 */
<<__Native>>
function watchman_callback_wrapper(string $sub_name): void;
} // namespace __SystemLib
