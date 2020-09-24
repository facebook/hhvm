<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace {

const int PAGELET_NOT_READY = 0;
const int PAGELET_READY = 0;
const int PAGELET_DONE = 0;

<<__PHPStdLib>>
function hphp_thread_type(): int;

<<__PHPStdLib>>
function pagelet_server_flush(): void;
<<__PHPStdLib>>
function pagelet_server_is_enabled(): bool;
<<__PHPStdLib>>
function pagelet_server_task_start(string $url, darray $headers = darray[], string $post_data = "", darray $files = darray[], int $timeout_seconds = 0): resource;
<<__PHPStdLib>>
function pagelet_server_task_status(resource $task): int;
<<__PHPStdLib>>
function pagelet_server_task_result(resource $task, inout $headers, inout $code, int $timeout_ms = 0): string;
<<__PHPStdLib>>
function pagelet_server_tasks_started(): int;

<<__PHPStdLib>>
function xbox_task_start(string $message): resource;
<<__PHPStdLib>>
function xbox_task_status(resource $task): bool;
<<__PHPStdLib>>
function xbox_task_result(resource $task, int $timeout_ms, inout $ret): int;
<<__PHPStdLib>>
function xbox_process_call_message(string $msg): mixed;
}

namespace HH {
function server_is_prepared_to_stop(): bool;
function server_is_stopping(): bool;
function server_health_level(): int;
function server_process_start_time(): int;
function server_uptime(): int;
}
