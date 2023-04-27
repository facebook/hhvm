<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function ob_start(
  HH\FIXME\MISSING_PARAM_TYPE $output_callback = null,
  int $chunk_size = 0,
  int $erase = PHP_OUTPUT_HANDLER_STDFLAGS,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function ob_clean(): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function ob_flush(): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function ob_end_clean(): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function ob_end_flush(): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function flush(): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function ob_get_clean(): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function ob_get_contents()[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function ob_get_flush(): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function ob_get_length()[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function ob_get_level()[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function ob_get_status(
  bool $full_status = false,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function ob_gzhandler(
  HH\FIXME\MISSING_PARAM_TYPE $buffer,
  HH\FIXME\MISSING_PARAM_TYPE $mode,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function ob_implicit_flush(bool $flag = true): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function ob_list_handlers()[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}

<<__PHPStdLib>>
function hphp_crash_log(
  string $name,
  string $value,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hphp_stats(string $name, int $value): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hphp_get_stats(
  string $name,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hphp_get_status()[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function hphp_get_iostatus()[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function hphp_set_iostatus_address(string $name): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hphp_get_timers(
  bool $get_as_float = true,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hphp_output_global_state(
  bool $serialize = true,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hphp_instruction_counter(
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function hphp_get_hardware_counters(
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function hphp_set_hardware_events(
  HH\FIXME\MISSING_PARAM_TYPE $events = null,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function hphp_clear_hardware_events(): HH\FIXME\MISSING_RETURN_TYPE {}

const int PHP_OUTPUT_HANDLER_START;
const int PHP_OUTPUT_HANDLER_WRITE;
const int PHP_OUTPUT_HANDLER_FLUSH;
const int PHP_OUTPUT_HANDLER_CLEAN;
const int PHP_OUTPUT_HANDLER_FINAL;
const int PHP_OUTPUT_HANDLER_CONT;
const int PHP_OUTPUT_HANDLER_END;

const int PHP_OUTPUT_HANDLER_CLEANABLE;
const int PHP_OUTPUT_HANDLER_FLUSHABLE;
const int PHP_OUTPUT_HANDLER_REMOVABLE;
const int PHP_OUTPUT_HANDLER_STDFLAGS;
