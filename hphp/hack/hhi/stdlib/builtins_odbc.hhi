<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function odbc_autocommit(resource $connection_id, ?bool $OnOff = false): bool;

<<__PHPStdLib>>
function odbc_commit(resource $connection_id): bool;

<<__PHPStdLib>>
function odbc_connect(string $dsn, string $user, string $password,
                      ?int $cursor_type=0): mixed; // resource or false on error

<<__PHPStdLib>>
function odbc_close(resource $connection_id): void;

<<__PHPStdLib>>
function odbc_error(?resource $connection_id = null): string;

<<__PHPStdLib>>
function odbc_errormsg(?resource $connection_id = null): string;

<<__PHPStdLib>>
function odbc_exec(resource $connection_id,
                   string $query_string,
                   ?int $flags=0): mixed;

<<__PHPStdLib>>
function odbc_execute(resource $result, ?array $parameters_array): bool;

<<__PHPStdLib>>
function odbc_fetch_array(resource $result, ?int $rownumber=0);

<<__PHPStdLib>>
function odbc_num_rows(?resource $result): int;

<<__PHPStdLib>>
function odbc_prepare(resource $connection_id, string $query_string): mixed;

<<__PHPStdLib>>
function odbc_rollback(resource $connection_id): bool;
