<?hh // decl
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

function array_fill<T>(int $start_index, int $num, T $value): array<T>;
// TODO make non-nullable once Thrift files are fixed
function chr(int $ascii): string;
function count(mixed $x, int $mode = COUNT_NORMAL): int; // count takes Countable or array. We'll need to hardcode this...
function dechex(int $number): string;
function fb_bsdiff(string $data1, string $data2): (array<int>, string, string);
function fb_bspatch(
  string $data1,
  array<int> $control,
  string $diff,
  string $extra,
): string;
function func_get_args(): array;
function implode(string $glue, $pieces): string; // could be Container<Stringish>
function explode(string $delimiter, ?Stringish $str, int $limit = 0x7FFFFFFF): array; // : array<string> & false for '' delimiter
function is_array(mixed $arg): bool;
function isset($x): bool;
function unset($x): void;
function ord(string $string): int;
function strip_tags(string $str, string $allowable_tags = ''): string;

function gzcompress(string $data, int $level = -1): mixed;
function gzdecode(string $data, int $length = PHP_INT_MAX): mixed;
function gzdeflate(string $data, int $level = -1): mixed;
function gzencode(string $data, int $level = -1): mixed;
function gzinflate(string $data, int $length = 0): mixed;
function gzuncompress(string $data, int $length = 0): mixed;

function intval($v, $base = 10): int;
function doubleval($v): float;
function floatval($v): float;
function strval($v): string;
function boolval($v): bool;

// enum helpers
// null $class means to use the get_called_class() context
function fb_get_enum_values(?string $class, bool $recurse): array;
// null $class means to use the get_called_class() context
function fb_get_enum_names(?string $class, bool $recurse): array;

function get_class_constants($class_name): array;

// the return value is an instance with class $class
// do **not** use this in your code, call newv() instead
function hphp_create_object<T>(string $class_name, array $argv): T;
