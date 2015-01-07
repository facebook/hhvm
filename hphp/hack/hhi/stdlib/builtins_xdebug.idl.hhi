<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

function xdebug_break(): bool;

function xdebug_call_class(): mixed; // calling class or false

function xdebug_call_file(): string;

function xdebug_call_line(): int;

function xdebug_call_function(): mixed; // function/method or false

function xdebug_code_coverage_started(): bool;

function xdebug_debug_zval(string $varname, ...): void;

function xdebug_debug_zval_stdout(string $varname, ...): void;

function xdebug_disable(): void;

function xdebug_dump_superglobals(): void;

function xdebug_enable(): void;

function xdebug_get_code_coverage(): array<string, array<int, int>>;

function xdebug_get_collected_errors(bool $clean = false): array<string>;

function xdebug_get_declared_vars(): array<string>;

function xdebug_get_function_stack(): array<array<string, mixed>>;

function xdebug_get_headers(): array<string>;

function xdebug_get_profiler_filename(): mixed; // string or false

function xdebug_get_stack_depth(): int;

function xdebug_get_tracefile_name(): mixed; // string or false

function xdebug_is_enabled(): bool;

function xdebug_memory_usage(): int;

function xdebug_peak_memory_usage(): int;

function xdebug_print_function_stack(string $message = 'user triggered',
                                     int $options = 0): void;

// Passing a value other than 0 for options is not supported in this
// implementation, and results in a fatal error.
function xdebug_start_code_coverage(int $options = 0): void;

function xdebug_start_error_collection(): void;

function xdebug_start_trace(
  mixed $trace_file = null,
  int $options = 0,
): mixed; // filename or false

function xdebug_stop_code_coverage(bool $cleanup = true): void;

function xdebug_stop_error_collection(): void;

function xdebug_stop_trace(): mixed; // filename or false

function xdebug_time_index(): float;

function xdebug_var_dump(mixed $var, ...): void;
