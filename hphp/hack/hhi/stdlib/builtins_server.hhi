<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int PAGELET_NOT_READY = 0;
const int PAGELET_READY = 0;
const int PAGELET_DONE = 0;

<<__PHPStdLib>>
function dangling_server_proxy_old_request();
<<__PHPStdLib>>
function pagelet_server_is_enabled();
<<__PHPStdLib>>
function pagelet_server_task_start($url, $headers = null, $post_data = null, $files = null, int $timeout_seconds = 0);
<<__PHPStdLib>>
function pagelet_server_task_status($task);
<<__PHPStdLib>>
function pagelet_server_task_result($task, &$headers, &$code, $timeout_ms = 0): string;
<<__PHPStdLib>>
function pagelet_server_flush();
<<__PHPStdLib>>
function xbox_send_message($msg, &$ret, $timeout_ms, $host = "localhost");
<<__PHPStdLib>>
function xbox_post_message($msg, $host = "localhost");
<<__PHPStdLib>>
function xbox_task_start($message);
<<__PHPStdLib>>
function xbox_task_status($task);
<<__PHPStdLib>>
function xbox_task_result($task, $timeout_ms, &$ret);
<<__PHPStdLib>>
function xbox_process_call_message($msg);
<<__PHPStdLib>>
function xbox_get_thread_timeout();
<<__PHPStdLib>>
function xbox_set_thread_timeout($timeout);
<<__PHPStdLib>>
function xbox_schedule_thread_reset();
<<__PHPStdLib>>
function xbox_get_thread_time();
