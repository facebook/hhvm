<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
define('PAGELET_NOT_READY', 0);
define('PAGELET_READY', 0);
define('PAGELET_DONE', 0);
function dangling_server_proxy_old_request() { }
function pagelet_server_is_enabled() { }
function pagelet_server_task_start($url, $headers = null, $post_data = null, $files = null) { }
function pagelet_server_task_status($task) { }
function pagelet_server_task_result($task, &$headers, &$code, $timeout_ms = 0) { }
function pagelet_server_flush() { }
function xbox_send_message($msg, &$ret, $timeout_ms, $host = "localhost") { }
function xbox_post_message($msg, $host = "localhost") { }
function xbox_task_start($message) { }
function xbox_task_status($task) { }
function xbox_task_result($task, $timeout_ms, &$ret) { }
function xbox_process_call_message($msg) { }
function xbox_get_thread_timeout() { }
function xbox_set_thread_timeout($timeout) { }
function xbox_schedule_thread_reset() { }
function xbox_get_thread_time() { }
