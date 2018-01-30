<?hh // decl   /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/**
 * This file provides type information for some of PHP's predefined functions
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

// isset, empty, and unset only look like functions to static analysis, but
// in fact act as special expression subtypes within the runtime
<<__Rx>>
function isset($x): bool;
<<__Rx>>
function empty($x): bool;
<<__Rx>>
function unset($x): void;

// freeze is a special function for mutability
function freeze($x) : void;

//
<<__PHPStdLib>>
function array_fill<T>(int $start_index, int $num, T $value): array<T>;
// TODO make non-nullable once Thrift files are fixed
<<__PHPStdLib>>
function chr(int $ascii): string;
<<__PHPStdLib>>
function count(mixed $x, int $mode = COUNT_NORMAL): int; // count takes Countable or array. We'll need to hardcode this...
<<__PHPStdLib>>
function dechex(int $number): string;
<<__Rx>>
function func_get_args(): array;
<<__PHPStdLib>>
function implode(string $glue, $pieces): string; // could be Container<Stringish>
<<__PHPStdLib>>
function explode(string $delimiter, ?Stringish $str, int $limit = 0x7FFFFFFF): array; // : array<string> & false for '' delimiter
<<__Rx>>
function is_array(mixed $arg): bool;
<<__Rx>>
function is_vec(mixed $arg): bool;
<<__Rx>>
function is_dict(mixed $arg): bool;
<<__Rx>>
function is_keyset(mixed $arg): bool;
<<__PHPStdLib>>
function ord(string $string): int;
<<__PHPStdLib>>
function strip_tags(string $str, string $allowable_tags = ''): string;

<<__PHPStdLib>>
function intval($v, $base = 10): int;
<<__PHPStdLib>>
function doubleval($v): float;
<<__PHPStdLib>>
function floatval($v): float;
<<__PHPStdLib>>
function strval($v): string;
<<__PHPStdLib>>
function boolval($v): bool;

<<__PHPStdLib>>
function get_class_constants($class_name): array;

// the return value is an instance with class $class
// do **not** use this in your code, call newv() instead
<<__PHPStdLib>>
function hphp_create_object<T>(string $class_name, array $argv): T;

namespace HH {
  // autoload-map
  function could_include($file): bool;
  function autoload_set_paths(
    \Indexish<string, \Indexish<string, string>> $map,
    string $root,
  ): bool;

  function set_frame_metadata(mixed $metadata): void;
}
