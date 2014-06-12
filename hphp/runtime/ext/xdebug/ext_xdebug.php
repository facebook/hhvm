<?hh
// TODO(#4489053) docs, override var_dump

<<__Native>>
function xdebug_break(): bool;

<<__Native>>
function xdebug_call_class(): string;

<<__Native>>
function xdebug_call_file(): string;

<<__Native>>
function xdebug_call_line(): int;

<<__Native>>
function xdebug_code_coverage_started(): bool;

<<__Native("ActRec")>>
function xdebug_debug_zval(string $varname, ...): void;

<<__Native("ActRec")>>
function xdebug_debug_zval_stdout(string $varname, ...): void;

<<__Native>>
function xdebug_disable(): void;

<<__Native>>
function xdebug_dump_superglobals(): void;

<<__Native>>
function xdebug_enable(): void;

<<__Native>>
function xdebug_get_code_coverage(): array<string, array<int, int>>;

<<__Native>>
function xdebug_get_collected_errors(bool $clean = false): array<string>;

<<__Native>>
function xdebug_get_declared_vars(): array<string>;

<<__Native>>
function xdebug_get_function_stack(): array<mixed>;

<<__Native>>
function xdebug_get_headers(): array<string>;

<<__Native>>
function xdebug_get_profiler_filename(): string;

<<__Native>>
function xdebug_get_stack_depth(): int;

<<__Native>>
function xdebug_get_tracefile_name(): string;

<<__Native>>
function xdebug_is_enabled(): bool;

<<__Native>>
function xdebug_memory_usage(): int;

<<__Native>>
function xdebug_peak_memory_usage(): int;

<<__Native>>
function xdebug_print_function_stack(string $message = 'user triggered',
                                     int $options = 0): void;

<<__Native>>
function xdebug_start_code_coverage(int $options = 0): void;

<<__Native>>
function xdebug_start_error_collection(): void;

<<__Native>>
function xdebug_start_trace(string $trace_file, int $options = 0): void;

<<__Native>>
function xdebug_stop_code_coverage(bool $cleanup = true): void;

<<__Native>>
function xdebug_stop_error_collection(): void;

<<__Native>>
function xdebug_stop_trace(): void;

<<__Native>>
function xdebug_time_index(): float;

<<__Native("ActRec")>>
function xdebug_var_dump(mixed $var, ...): void;
