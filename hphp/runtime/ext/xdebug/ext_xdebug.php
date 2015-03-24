<?hh

/**
 * This function makes the debugger break on the specific line as if a normal
 * file/line breakpoint was set on this line.
 *
 * @return bool - True on success, false on failure
 */
<<__Native>>
function xdebug_break(): bool;

/**
 * This function returns the name of the class from which the current
 * function/method was called from.
 *
 * @return mixed - Returns the calling class or false if called at the top level
 */
<<__Native("NoFCallBuiltin")>>
function xdebug_call_class(): mixed;

/**
 * This function returns the filename that contains the function/method that
 * called the current function/method.
 *
 * @return string - Returns the calling file.
 */
<<__Native("NoFCallBuiltin")>>
function xdebug_call_file(): string;

/**
 * This function returns the line number that contains the function/method that
 * called the current function/method.
 *
 * @return int - Returns the calling line
 */
<<__Native("NoFCallBuiltin")>>
function xdebug_call_line(): int;

/*
 * This function returns the name of the function/method from which the current
 * function/method was called from.
 *
 * @return mixed- Returns the calling function/method or false if called at the
 *                top level
 */
<<__Native("NoFCallBuiltin")>>
function xdebug_call_function(): mixed;

/**
 * Returns whether code coverage has been started.
 *
 * @return bool - Returns whether code coverage is active.
 */
<<__Native>>
function xdebug_code_coverage_started(): bool;

<<__Native("ActRec")>>
function xdebug_debug_zval(string $varname, ...): void;

<<__Native("ActRec")>>
function xdebug_debug_zval_stdout(string $varname, ...): void;

/**
 * Disable showing stack traces on error conditions.
 */
<<__Native>>
function xdebug_disable(): void;

<<__Native>>
function xdebug_dump_superglobals(): void;

/**
 * Enable showing stack traces on error conditions.
 */
<<__Native>>
function xdebug_enable(): void;

/**
 * Returns a structure which contains information about which lines were
 * executed in your script (including include files).
 *
 * @return array - Returns code coverage information
 */
<<__Native>>
function xdebug_get_code_coverage(): array<string, array<int, int>>;

<<__Native>>
function xdebug_get_collected_errors(bool $clean = false): array<string>;

/**
 * Returns an array where each element is a variable name which is defined in
 * the current scope.
 *
 * @return array - Returns the declared variables
 */
<<__Native("NoFCallBuiltin")>>
function xdebug_get_declared_vars(): array<string>;

/**
 * Returns an array which resembles the stack trace up to this point.
 *
 * Known differences with PHP5 xdebug:
 *   - In PHP5 xdebug the function arguments, if collect_params is nonzero,
 *     are provided as an array<string, string>. The key is the argument name
 *     and the value is the xdebug-specific serialization of the argument
 *     value. This serialization matches the output of xdebug_var_dump, but
 *     isn't useful because there isn't a way to unserialize the string.
 *
 *     This implementation provides arguments as an array<string, mixed> where
 *     the key is the argument name and the value is the actual argument.
 *
 * @return array - Returns information about the stack
 */
function xdebug_get_function_stack(): array<array<string, mixed>> {
  $st = \__SystemLib\xdebug_get_function_stack();

  // Make the trace match php5 xdebug:
  //  Remove the call to xdebug_get_function_stack, add a file and
  //  line to the main frame, and change args -> params,
  $st[0]["line"] = 0;
  $st[0]["file"] = $st[1]["file"]; // Frame 1 is within pseudo-main
  array_pop($st);
  foreach ($st as &$frame) {
    $frame["params"] = $frame["args"];
    unset($frame["args"]);
  }
  return $st;
}

<<__Native>>
function xdebug_get_headers(): array<string>;

/**
 * Returns the name of the file which is used to save profile information to.
 *
 * @return mixed - Returns the profile information filename or false if
 *                 profiling is disabled.
 */
<<__Native>>
function xdebug_get_profiler_filename(): mixed;

/**
 * Returns the stack depth level. The main body of a script is level 0 and each
 * include and/or function call adds one to the stack depth level.
 *
 * @return int - Returns the current stack depth level
 */
<<__Native>>
function xdebug_get_stack_depth(): int;

/**
 * Returns the name of the file which is used to trace the output of this script
 * too. This is useful when xdebug.auto_trace is enabled.
 *
 * @return mixed - Returns the name of the function trace file or false if
 *                 tracing isn't enabled.
 */
<<__Native>>
function xdebug_get_tracefile_name(): mixed;

/**
 * Return whether stack traces would be shown in case of an error or not.
 */
<<__Native>>
function xdebug_is_enabled(): bool;

/**
 * Returns the current amount of memory the script uses.
 *
 * @return int - Returns the current memory usage
 */
<<__Native>>
function xdebug_memory_usage(): int;

/**
 * Returns the maximum amount of memory the script used until now.
 *
 * @return int - Returns the peak memory usage
 */
<<__Native>>
function xdebug_peak_memory_usage(): int;

<<__Native>>
function xdebug_print_function_stack(string $message = 'user triggered',
                                     int $options = 0): void;

/**
 * This function starts gathering the information for code coverage. The
 * information that is collected consists of an two dimensional array with
 * as primary index the executed filename and as secondary key the line
 * number. The value in the elements represents how many times a line
 * has been executed.
 *
 * Note that this is different from xdebug. Xdebug only records whether
 * or not a line has been excuted, this implementation includes the number
 * of times a line was executed.
 *
 * @param options - This is a bitmask that xdebug uses for options
 *                  XDEBUG_CC_UNUSED and XDEBUG_CC_DEAD_CODE. Neither of these
 *                  options is supported in this implementation, and if provided
 *                  a fatal error will be thrown.
 */
<<__Native("NoFCallBuiltin")>>
function xdebug_start_code_coverage(int $options = 0): void;

<<__Native>>
function xdebug_start_error_collection(): void;

/**
 * Start tracing function calls from this point to the file in the trace_file
 * parameter. If no filename is given, then the trace file will be placed
 * in the directory as configured by the xdebug.trace_output_dir setting.
 * In case a file name is given as first parameter, the name is relative to the
 * current working directory. This current working directory might be different
 * than you expect it to be, so please use an absolute path in case you specify
 * a file name. Use the PHP function getcwd() to figure out what the current
 * working directory is.
 *
 * The name of the trace file is "{trace_file}.xt". If xdebug.auto_trace is
 * enabled, then the format of the filename is "{filename}.xt" where the
 * "{filename}" part depends on the xdebug.trace_output_name setting. The
 * options parameter is a bitfield; currently there are four options:
 *   XDEBUG_TRACE_APPEND (1)
 *    makes the trace file open in append mode rather than overwrite mode
 *   XDEBUG_TRACE_COMPUTERIZED (2)
 *    creates a trace file with the format as described under 1
 *    "xdebug.trace_format".
 *   XDEBUG_TRACE_HTML (4)
 *    creates a trace file as an HTML table
 *   XDEBUG_TRACE_NAKED_FILENAME (8)
 *    Normally, Xdebug always adds ".xt" to the end of the filename that you
 *    pass in as first argument to this function. With the
 *    XDEBUG_TRACE_NAKED_FILENAME flag set, ".xt" is not added.
 *
 *  The settings xdebug.collect_includes, xdebug.collect_params and
 *  xdebug.collect_return influence what information is logged to the trace
 *  file and the setting xdebug.trace_format influences the format of the trace
 *  file.
 *
 * @return mixed - The filename returned by xdebug_get_tracefile_name() or false
 *                 on failure.
 */
<<__Native("NoFCallBuiltin")>>
function xdebug_start_trace(mixed $trace_file = null, int $options = 0): mixed;

/**
 * This function stops collecting information, the information in memory will
 * be destroyed. If you pass "false" as argument, then the code coverage
 * information will not be destroyed so that you can resume the gathering of
 * information with the xdebug_start_code_coverage() function again.
 *
 * @param cleanup - If true, code coverage collected information is destroyed.
 */
<<__Native>>
function xdebug_stop_code_coverage(bool $cleanup = true): void;

<<__Native>>
function xdebug_stop_error_collection(): void;

/**
 * Stop tracing function calls and closes the trace file.
 *
 * @return mixed - If tracing was started, the file the trace was saved to.
 *                 Otherwise, false.
 */
<<__Native("NoFCallBuiltin")>>
function xdebug_stop_trace(): mixed;

/**
 * Returns the current time index since the starting of the script in seconds.
 *
 * @return float - Returns the current time index
 */
<<__Native>>
function xdebug_time_index(): float;

<<__Native>>
function xdebug_var_dump(mixed $var, ...$argv): void;

/**
 * Should only be needed when request parameters are being manipulated, such
 * as in unit tests. Checks to see if any of the trigger $_GET, $_POST, or
 * $_COOKIE variables are set, and starts the xdebug profiler if necessary.
 */
<<__Native>>
function _xdebug_check_trigger_vars(): void;

namespace __SystemLib {
  /**
   * Native helper for xdebug_get_function_stack
   *
   * @return array - The not formatted stack trace
   */
  <<__Native>>
  function xdebug_get_function_stack(): array<array<string, mixed>>;
}
