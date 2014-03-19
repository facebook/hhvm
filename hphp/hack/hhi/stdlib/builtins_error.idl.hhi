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
function debug_backtrace(int $options = 1, int $limit = 0) { }
function debug_print_backtrace() { }
function error_get_last() { }
function error_log($message, $message_type = 0, $destination = null, $extra_headers = null) { }
function error_reporting($level = null) { }
function restore_error_handler() { }
function restore_exception_handler() { }
function set_error_handler($error_handler, $error_types = E_ALL) { }
function set_exception_handler($exception_handler) { }
function hphp_set_error_page($page) { }
function hphp_throw_fatal_error($error_msg) { }
function hphp_clear_unflushed() { }
function hphp_debug_caller_info() { }
function trigger_error($error_msg, $error_type = E_USER_NOTICE) { }
function user_error($error_msg, $error_type = E_USER_NOTICE) { }
