<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

function debug_backtrace(
  int $options = DEBUG_BACKTRACE_PROVIDE_OBJECT,
  int $limit = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function debug_print_backtrace(int $options = 0, int $limit = 0): void {}
<<__PHPStdLib>>
function error_get_last(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function error_log(
  string $message,
  int $message_type = 0,
  HH\FIXME\MISSING_PARAM_TYPE $destination = null,
  HH\FIXME\MISSING_PARAM_TYPE $extra_headers = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function error_reporting(
  HH\FIXME\MISSING_PARAM_TYPE $level = null,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function restore_error_handler(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function restore_exception_handler(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function set_error_handler(
  HH\FIXME\MISSING_PARAM_TYPE $error_handler,
  int $error_types = E_ALL,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function set_exception_handler(
  HH\FIXME\MISSING_PARAM_TYPE $exception_handler,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hphp_set_error_page(string $page): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hphp_throw_fatal_error(string $error_msg)[]: noreturn;
<<__PHPStdLib>>
function hphp_clear_unflushed(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hphp_debug_caller_info()[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hphp_debug_backtrace_hash(int $options = 0)[leak_safe]: int;
<<__PHPStdLib>>
function trigger_error(
  string $error_msg,
  int $error_type = E_USER_NOTICE,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function trigger_sampled_error(
  string $error_msg,
  int $sample_rate,
  int $error_type = E_USER_NOTICE,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function user_error(
  string $error_msg,
  int $error_type = E_USER_NOTICE,
): HH\FIXME\MISSING_RETURN_TYPE;
