<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function ob_start($output_callback = null, $chunk_size = 0, $erase = true) { }
<<__PHPStdLib>>
function ob_clean() { }
<<__PHPStdLib>>
function ob_flush() { }
<<__PHPStdLib>>
function ob_end_clean() { }
<<__PHPStdLib>>
function ob_end_flush() { }
<<__PHPStdLib>>
function flush() { }
<<__PHPStdLib>>
function ob_get_clean() { }
<<__PHPStdLib>>
function ob_get_contents() { }
<<__PHPStdLib>>
function ob_get_flush() { }
<<__PHPStdLib>>
function ob_get_length() { }
<<__PHPStdLib>>
function ob_get_level() { }
<<__PHPStdLib>>
function ob_get_status($full_status = false) { }
<<__PHPStdLib>>
function ob_gzhandler($buffer, $mode) { }
<<__PHPStdLib>>
function ob_implicit_flush($flag = true) { }
<<__PHPStdLib>>
function ob_list_handlers() { }
<<__PHPStdLib>>
function output_add_rewrite_var($name, $value) { }
<<__PHPStdLib>>
function output_reset_rewrite_vars() { }
<<__PHPStdLib>>
function hphp_crash_log($name, $value) { }
<<__PHPStdLib>>
function hphp_stats($name, $value) { }
<<__PHPStdLib>>
function hphp_get_stats($name) { }
<<__PHPStdLib>>
function hphp_get_status() { }
<<__PHPStdLib>>
function hphp_get_iostatus() { }
<<__PHPStdLib>>
function hphp_set_iostatus_address($name) { }
<<__PHPStdLib>>
function hphp_get_timers($get_as_float = true) { }
<<__PHPStdLib>>
function hphp_output_global_state($serialize = true) { }
<<__PHPStdLib>>
function hphp_instruction_counter() { }
<<__PHPStdLib>>
function hphp_get_hardware_counters() { }
<<__PHPStdLib>>
function hphp_set_hardware_events($events = null) { }
<<__PHPStdLib>>
function hphp_clear_hardware_events() { }

const int PHP_OUTPUT_HANDLER_START = 0;
const int PHP_OUTPUT_HANDLER_WRITE = 0;
const int PHP_OUTPUT_HANDLER_FLUSH = 0;
const int PHP_OUTPUT_HANDLER_CLEAN = 0;
const int PHP_OUTPUT_HANDLER_FINAL = 0;
const int PHP_OUTPUT_HANDLER_CONT = 0;
const int PHP_OUTPUT_HANDLER_END = 0;

const int PHP_OUTPUT_HANDLER_CLEANABLE = 0;
const int PHP_OUTPUT_HANDLER_FLUSHABLE = 0;
const int PHP_OUTPUT_HANDLER_REMOVABLE = 0;
const int PHP_OUTPUT_HANDLER_STDFLAGS = 0;
