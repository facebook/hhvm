<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

function debug_backtrace(int $options = DEBUG_BACKTRACE_PROVIDE_OBJECT, int $limit = 0);
<<__PHPStdLib>>
function debug_print_backtrace(int $options = 0, int $limit = 0): void { }
<<__PHPStdLib>>
function error_get_last();
<<__PHPStdLib>>
function error_log(string $message, int $message_type = 0, $destination = null, $extra_headers = null);
<<__PHPStdLib>>
function error_reporting($level = null);
<<__PHPStdLib>>
function restore_error_handler();
<<__PHPStdLib>>
function restore_exception_handler();
<<__PHPStdLib>>
function set_error_handler($error_handler, int $error_types = E_ALL);
<<__PHPStdLib>>
function set_exception_handler($exception_handler);
<<__PHPStdLib>>
function hphp_set_error_page(string $page);
<<__PHPStdLib>>
function hphp_throw_fatal_error(string $error_msg);
<<__PHPStdLib>>
function hphp_clear_unflushed();
<<__PHPStdLib>>
function hphp_debug_caller_info();
<<__PHPStdLib>>
function hphp_debug_backtrace_hash(int $options = 0);
<<__PHPStdLib>>
function trigger_error(string $error_msg, int $error_type = E_USER_NOTICE)[];
<<__PHPStdLib>>
function trigger_sampled_error(string $error_msg, int $sample_rate, int $error_type = E_USER_NOTICE)[];
<<__PHPStdLib>>
function user_error(string $error_msg, int $error_type = E_USER_NOTICE);
