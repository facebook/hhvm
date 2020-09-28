<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int THRIFT_MARK_LEGACY_ARRAYS = 1 << 0;

<<__PHPStdLib>>
function thrift_protocol_write_binary($transportobj, string $method_name, int $msgtype, $request_struct, int $seqid, bool $strict_write, bool $oneway = false);
<<__PHPStdLib>>
function thrift_protocol_read_binary($transportobj, string $obj_typename, bool $strict_read, int $options = 0);
<<__PHPStdLib>>
function thrift_protocol_set_compact_version(int $version);
<<__PHPStdLib>>
function thrift_protocol_write_compact($transportobj, string $method_name, int $msgtype, $request_struct, int $seqid, bool $oneway = false);
<<__PHPStdLib>>
function thrift_protocol_read_compact($transportobj, string $obj_typename, int $options = 0);
<<__PHPStdLib>>
function thrift_protocol_read_compact_struct($transportobj, string $obj_typename, int $options = 0);
<<__PHPStdLib>>
function thrift_protocol_read_binary_struct($transportobj, string $obj_typename, int $options = 0);
