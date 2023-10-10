<?hh

namespace {

/**
 * Return thread type. See enum class ThreadType.
 *
 * @return int - thread type. Returns -1 if unknown.
 */
<<__Native>>
function hphp_thread_type(): int;

/**
 * Whether pagelet server is enabled or not. Please read server documentation
 *   for what a pagelet server is.
 *
 * @return bool - TRUE if it's enabled, FALSE otherwise.
 *
 */
<<__Native>>
function pagelet_server_is_enabled(): bool;

/**
 * Processes a pagelet server request.
 *
 * @param string $url - The URL we're running this pagelet with.
 * @param array $headers - HTTP headers to send to the pagelet.
 * @param string $post_data - POST data to send.
 * @param array $files - Array for the pagelet.
 *
 * @return resource - An object that can be used with
 *   pagelet_server_task_status() or pagelet_server_task_result().
 *
 */
<<__Native>>
function pagelet_server_task_start(string $url,
                                   dict<arraykey, string> $headers = dict[],
                                   string $post_data = "",
                                   dict<arraykey, string> $files = dict[],
                                   int $timeout_seconds = 0): resource;

/**
 * Checks finish status of a pagelet task.
 *
 * @param resource $task - The pagelet task handle returned from
 *   pagelet_server_task_start().
 *
 * @return int - PAGELET_NOT_READY if there is no data available,
 *   PAGELET_READY if (partial) data is available from pagelet_server_flush(),
 *   and PAGELET_DONE if the pagelet request is done.
 *
 */
<<__Native>>
function pagelet_server_task_status(resource $task): int;

/**
 * Block and wait until pagelet task finishes or times out.
 *
 * @param resource $task - The pagelet task handle returned from
 *   pagelet_server_task_start().
 * @param mixed $headers - HTTP response headers.
 * @param mixed $code - HTTP response code. Set to -1 in the event of a
 *   timeout.
 * @param int $timeout_ms - How many milliseconds to wait. A timeout of zero
 *   is interpreted as an infinite timeout.
 *
 * @return string - HTTP response from the pagelet.
 *
 */
<<__Native("NoFCallBuiltin")>>
function pagelet_server_task_result(
  resource $task,
  <<__OutOnly('varray')>>
  inout mixed $headers,
  <<__OutOnly('KindOfInt64')>>
  inout mixed $code,
  int $timeout_ms = 0,
): string;

/**
 * Return the number of pagelet tasks started during this request.
 *
 * @return int - Number of pagelet tasks started.
 */
<<__Native>>
function pagelet_server_tasks_started(): int;

/**
 * Flush all the currently buffered output, so that the main thread can read
 *   it with pagelet_server_task_result(). This is only meaningful in a pagelet
 *   thread.
 *
 */
<<__Native>>
function pagelet_server_flush(): void;

/**
 * Determine whether or not the pagelet thread we are executing on has finished
 * and closed its output buffer.
 */
<<__Native>>
function pagelet_server_is_done(): bool;

/**
 * Starts a local xbox task.
 *
 * @param string $message - A message to send to xbox's message processing
 *   function.
 *
 * @return resource - A task handle xbox_task_status() and xbox_task_result()
 *   can use.
 *
 */
<<__Native>>
function xbox_task_start(string $message): resource;

/**
 * Checks an xbox task's status.
 *
 * @param resource $task - The xbox task object created by xbox_task_start().
 *
 * @return bool - TRUE if finished, FALSE otherwise.
 *
 */
<<__Native>>
function xbox_task_status(resource $task): bool;

/**
 * Block and wait for xbox task's result.
 *
 * @param resource $task - The xbox task object created by xbox_task_start().
 * @param int $timeout_ms - How many milli-seconds to wait.
 * @param mixed $ret - xbox message processing function's return value.
 *
 * @return int - Response code following HTTP's responses. For example, 200
 *   for success and 500 for server error.
 *
 */
<<__Native>>
function xbox_task_result(
  resource $task,
  int $timeout_ms,
  <<__OutOnly>> inout mixed $ret,
): int;

/**
 * This function is invoked by the xbox facility to start an xbox call task.
 *   This function is not intended to be called directly by user code.
 *
 * @param string $msg - The call message.
 *
 * @return mixed - The return value of the xbox call task.
 *
 */
<<__Native>>
function xbox_process_call_message(string $msg): mixed;

} // root namespace

namespace HH {
/**
 * Whether the server is going to stop soon.
 *
 * @return bool - True if server is going to stop soon, False if
 * server is not running, or is running without a schedule to stop.
 *
 */
<<__Native>>
function server_is_stopping(): bool;

/**
 * Whether the server is prepared to stop.  This is different from
 * 'server_is_stopping', because the server has not received the 'stop' command,
 * and is not scheduled to stop.  It is still fully functional, able to handle
 * requests for an indefinite amount of time, and should be considered healthy.
 * This is just a hint used during server update.
 *
 * @return bool - True if server has received 'prepare-to-stop' command from the
 * admin port in the past 'RuntimeOption::ServerPrepareToStop' seconds; False
 * if server is not running, or has not received instructions to prepare to
 * stop.
 *
 */
<<__Native>>
function server_is_prepared_to_stop(): bool;

/**
 * Return the health level of the server in the range of 0~100.
 *
 * @return int - 100 if the server is very healthy, and 0 if the
 * server should not receive any more request.
 *
 */
<<__Native>>
function server_health_level(): int;

/**
 * Returns the time that the http server has been accepting connections.
 *
 * @return int - number of seconds the server has been running.  -1 if
 * server is not started.
 *
 */
<<__Native>>
function server_uptime(): int;

/**
 * Returns the timestamp when the http server process was started.
 *
 * @return int - number of seconds since epoch when process started.  0 if
 * server is not started.
 *
 */
<<__Native>>
function server_process_start_time(): int;

}
