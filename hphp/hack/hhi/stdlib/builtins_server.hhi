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
function pagelet_server_task_start(string $url, darray $headers = darray[], string $post_data = "", darray $files = darray[], int $timeout_seconds = 0);
<<__PHPStdLib>>
function pagelet_server_task_status(resource $task);
<<__PHPStdLib>>
function pagelet_server_task_result(resource $task, inout $headers, inout $code, int $timeout_ms = 0): string;
<<__PHPStdLib>>
function pagelet_server_flush();
<<__PHPStdLib>>
function xbox_send_message(string $msg, inout $ret, int $timeout_ms, string $host = "localhost");
<<__PHPStdLib>>
function xbox_post_message(string $msg, string $host = "localhost");
<<__PHPStdLib>>
function xbox_task_start(string $message);
<<__PHPStdLib>>
function xbox_task_status(resource $task);
<<__PHPStdLib>>
function xbox_task_result(resource $task, int $timeout_ms, inout $ret);
<<__PHPStdLib>>
function xbox_process_call_message(string $msg);
<<__PHPStdLib>>
function xbox_get_thread_timeout();
<<__PHPStdLib>>
function xbox_set_thread_timeout(int $timeout);
<<__PHPStdLib>>
function xbox_schedule_thread_reset();
<<__PHPStdLib>>
function xbox_get_thread_time();
