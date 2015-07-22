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

function odbc_autocommit(resource $connection_id, ?bool $OnOff = false): bool;

function odbc_commit(resource $connection_id): bool;

function odbc_connect(string $dsn, string $user, string $password,
                      ?int $cursor_type=0): mixed; // resource or false on error

function odbc_close(resource $connection_id): void;

function odbc_error(?resource $connection_id = null): string;

function odbc_errormsg(?resource $connection_id = null): string;

function odbc_exec(resource $connection_id,
                   string $query_string,
                   ?int $flags=0): mixed;

function odbc_execute(resource $result, ?array $parameters_array): bool;

function odbc_fetch_array(resource $result, ?int $rownumber=0);

function odbc_num_rows(?resource $result): int;

function odbc_prepare(resource $connection_id, string $query_string): mixed;

function odbc_rollback(resource $connection_id): bool;
