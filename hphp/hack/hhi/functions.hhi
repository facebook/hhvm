<?hh   /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

/**
 * This file provides type information for some of PHP's predefined functions
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

// isset, and unset only look like functions to static analysis, but
// in fact act as special expression subtypes within the runtime
namespace {
<<__Rx>>
function isset(<<__MaybeMutable>> $x): bool;
<<__Rx>>
function unset(<<__MaybeMutable>> $x): void;
//
<<__PHPStdLib, __Rx>>
function array_fill<T>(int $start_index, int $num, T $value): array<T>;
// TODO make non-nullable once Thrift files are fixed
<<__PHPStdLib, __Rx>>
function chr(int $ascii): string;
<<__PHPStdLib, __Rx>>
function count(<<__MaybeMutable>> mixed $x, int $mode = COUNT_NORMAL): int; // count takes Countable or array. We'll need to hardcode this...
<<__PHPStdLib, __Rx>>
function dechex(int $number): string;
<<__PHPStdLib, __Rx>>
function implode(string $glue, <<__MaybeMutable>> $pieces): string; // could be Container<Stringish>
<<__PHPStdLib, __Rx>>
function explode(string $delimiter, string $str, int $limit = 0x7FFFFFFF): array; // : array<string> & false for '' delimiter
<<__Rx>>
function is_array(<<__MaybeMutable>> mixed $arg): bool;
}

namespace HH {
<<__Rx>>
function is_vec(<<__MaybeMutable>> mixed $arg): bool;
<<__Rx>>
function is_dict(<<__MaybeMutable>> mixed $arg): bool;
<<__Rx>>
function is_keyset(<<__MaybeMutable>> mixed $arg): bool;
}

namespace {
<<__PHPStdLib, __Rx>>
function ord(string $string): int;
<<__PHPStdLib>>
function strip_tags(string $str, string $allowable_tags = ''): string;

<<__PHPStdLib, __Rx>>
function intval($v, int $base = 10): int;
<<__PHPStdLib, __Rx>>
function doubleval($v): float;
<<__PHPStdLib, __Rx>>
function floatval($v): float;
<<__PHPStdLib, __Rx>>
function strval($v): string;
<<__PHPStdLib, __Rx>>
function boolval($v): bool;

<<__PHPStdLib>>
function get_class_constants(string $class_name): array;

// the return value is an instance with class $class
// do **not** use this in your code, call newv() instead
<<__PHPStdLib>>
function hphp_create_object<T>(string $class_name, varray<mixed> $argv): T;
}

namespace HH {
  // autoload-map
  function could_include($file): bool;
  function autoload_is_native(): bool;
  function autoload_set_paths(
    KeyedContainer<string, KeyedContainer<string, string>> $map,
    string $root,
  ): bool;

  /**
   * Return all paths currently known to the autoloader.
   *
   * This may or may not be all the paths in your repo. If you call
   * `HH\autoload_set_paths()` with a callback and expect that callback to
   * lazily load paths as it sees new symbols, this function will only return
   * all paths which we have seen during this request.
   *
   * If native autoloading is enabled, or if every path passed to
   * `HH\autoload_set_paths()` was a valid path with all symlinks dereferenced,
   * then each path returned will be an absolute canonical path, with all
   * symlinks dereferenced.
   *
   * Throws InvalidOperationException if autoloading is disabled.
   */
  function autoload_get_paths(): Container<string>;

  /**
   * Get the path which uniquely defines the given symbol.
   *
   * Returns an absolute canonical path with all symlinks dereferenced.
   *
   * Throws InvalidOperationException if native autoloading is disabled.
   */
  function autoload_type_to_path(string $type): ?string;
  function autoload_function_to_path(string $function): ?string;
  function autoload_constant_to_path(string $constant): ?string;
  function autoload_type_alias_to_path(string $type_alias): ?string;

  /**
   * Get the types defined in the given path.
   *
   * The path may be relative to the repo root or absolute. But this function
   * will not dereference symlinks for you, so providing a path with symlinks
   * may cause this function to return an empty vec when you expected results.
   *
   * Throws InvalidOperationException if native autoloading is disabled.
   */
  function autoload_path_to_types(string $path): vec<classname<mixed>>;
  function autoload_path_to_functions(string $path): vec<string>;
  function autoload_path_to_constants(string $path): vec<string>;
  function autoload_path_to_type_aliases(string $path): vec<string>;

  newtype ParseTree = darray<string, mixed>;
  function ffp_parse_string(string $program): ParseTree;

  function clear_static_memoization(?string $cls, ?string $func = null) : bool;
  function clear_lsb_memoization(string $cls, ?string $func = null) : bool;
  function clear_instance_memoization($obj) : bool;

  <<__Rx>>
  function is_list_like(<<__MaybeMutable>> mixed $arg): bool;
  <<__Rx>>
  function is_class_meth(<<__MaybeMutable>> mixed $arg): bool;
  <<__Rx>>
  function is_meth_caller(<<__MaybeMutable>> mixed $arg): bool;
  <<__Rx>>
  function is_fun(<<__MaybeMutable>> mixed $arg): bool;

  <<__Rx>>
  function set_frame_metadata(mixed $metadata): void;

  // Code coverage
  function enable_per_file_coverage(keyset<string> $files): void;
  function disable_per_file_coverage(keyset<string> $files): void;
  function get_files_with_coverage(): keyset<string>;
  function get_coverage_for_file(string $file): vec<int>;
  function clear_coverage_for_file(string $file): void;
  function disable_all_coverage(): void;
  function get_all_coverage_data(): dict<string, vec<int>>;
  function clear_all_coverage_data(): void;
}
