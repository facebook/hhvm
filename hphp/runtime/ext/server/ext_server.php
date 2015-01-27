<?hh

/**
 * Return thread type. See enum class ThreadType.
 *
 * @return int - thread type. Returns -1 if unknown.
 */
<<__HipHopSpecific, __Native>>
function hphp_thread_type(): int;

/**
 * When I'm running a newer version of the server software and I'm getting an
 *   HTTP request that's from old version of a web page, proxy it to a local
 *   instance that is still running or dangling just for handling old version of
 *   requests. Please read server documentation for more details.
 *
 * @return bool - TRUE if successful, FALSE otherwise.
 *
 */
<<__HipHopSpecific, __Native>>
function dangling_server_proxy_old_request(): bool;

/**
 * Whether pagelet server is enabled or not. Please read server documentation
 *   for what a pagelet server is.
 *
 * @return bool - TRUE if it's enabled, FALSE otherwise.
 *
 */
<<__HipHopSpecific, __Native>>
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
<<__HipHopSpecific, __Native>>
function pagelet_server_task_start(string $url,
                                   array $headers = [],
                                   string $post_data = "",
                                   array $files = []): resource;

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
<<__HipHopSpecific, __Native>>
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
<<__HipHopSpecific, __Native>>
function pagelet_server_task_result(resource $task,
                                    mixed &$headers,
                                    mixed &$code,
                                    int $timeout_ms = 0): string;

/**
 * Return the number of pagelet tasks started during this request.
 *
 * @return int - Number of pagelet tasks started.
 */
<<__HipHopSpecific, __Native>>
function pagelet_server_tasks_started(): int;

/**
 * Flush all the currently buffered output, so that the main thread can read
 *   it with pagelet_server_task_result(). This is only meaningful in a pagelet
 *   thread.
 *
 */
<<__HipHopSpecific, __Native>>
function pagelet_server_flush(): void;

/**
 * Sends an xbox message and waits for response. Please read server
 *   documentation for what an xbox is.
 *
 * @param string $msg - The message.
 * @param mixed $ret - The response.
 * @param int $timeout_ms - How many milli-seconds to wait.
 * @param string $host - Which machine to send to.
 *
 * @return bool - TRUE if successful, FALSE otherwise.
 *
 */
<<__HipHopSpecific, __Native>>
function xbox_send_message(string $msg,
                           mixed &$ret,
                           int $timeout_ms,
                           string $host = "localhost"): bool;

/**
 * Posts an xbox message without waiting. Please read server documentation for
 *   more details.
 *
 * @param string $msg - The response.
 * @param string $host - Which machine to post to.
 *
 * @return bool - TRUE if successful, FALSE otherwise.
 *
 */
<<__HipHopSpecific, __Native>>
function xbox_post_message(string $msg, string $host = "localhost"): bool;

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
<<__HipHopSpecific, __Native>>
function xbox_task_start(string $message): resource;

/**
 * Checks an xbox task's status.
 *
 * @param resource $task - The xbox task object created by xbox_task_start().
 *
 * @return bool - TRUE if finished, FALSE otherwise.
 *
 */
<<__HipHopSpecific, __Native>>
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
<<__HipHopSpecific, __Native>>
function xbox_task_result(resource $task, int $timeout_ms, mixed &$ret): int;

/**
 * This function is invoked by the xbox facility to start an xbox call task.
 *   This function is not intended to be called directly by user code.
 *
 * @param string $msg - The call message.
 *
 * @return mixed - The return value of the xbox call task.
 *
 */
<<__HipHopSpecific, __Native>>
function xbox_process_call_message(string $msg): mixed;

/**
 * Gets the timeout (maximum duration), in seconds, of the current xbox
 *   thread. Throws for non-xbox threads.
 *
 * @return int - The current timeout (maximum duration).
 *
 */
<<__HipHopSpecific, __Native>>
function xbox_get_thread_timeout(): int;

/**
 * Sets the timeout (maximum duration), in seconds, of the current xbox
 *   thread. The xbox thread would reset when this amount of time has passed
 *   since the previous reset. Throws for non-xbox threads.
 *
 * @param int $timeout - The new timeout (maximum duration).
 *
 */
<<__HipHopSpecific, __Native>>
function xbox_set_thread_timeout(int $timeout): void;

/**
 * Schedules a reset of the current xbox thread, when the next request comes
 *   in. Throws for non-xbox threads.
 *
 */
<<__HipHopSpecific, __Native>>
function xbox_schedule_thread_reset(): void;

/**
 * Returns the time that the current xbox thread has been running without a
 *   reset, in seconds, and throws for non-xbox threads.
 *
 * @return int - The time that the current xbox thread has been running
 *   without a reset.
 *
 */
<<__HipHopSpecific, __Native>>
function xbox_get_thread_time(): int;
