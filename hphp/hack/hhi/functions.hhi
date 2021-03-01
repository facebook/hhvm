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
function isset($x)[]: bool;
function unset($x)[]: void;
//
<<__PHPStdLib>>
function array_fill<T>(int $start_index, int $num, T $value)[]: varray_or_darray<T>;
// TODO make non-nullable once Thrift files are fixed
<<__PHPStdLib>>
function chr(int $ascii)[]: string;
<<__PHPStdLib>>
function count(mixed $x, int $mode = COUNT_NORMAL)[]: int; // count takes Countable or array. We'll need to hardcode this...
<<__PHPStdLib>>
function dechex(int $number)[]: string;
<<__PHPStdLib>>
function implode(string $glue, $pieces)[]: string; // could be Container<Stringish>
<<__PHPStdLib>>
function explode(string $delimiter, string $str, int $limit = 0x7FFFFFFF)[]: varray<string>; // : array<string> & false for '' delimiter
}

namespace HH {
function is_vec(mixed $arg)[]: bool;
function is_dict(mixed $arg)[]: bool;
function is_keyset(mixed $arg)[]: bool;

/**
 * @returns True if `$arg` is a `varray`, `darray`, `dict`, `vec`, or `keyset`.
 * Otherwise returns false.
 */
function is_any_array(mixed $arg)[]: bool;
}

namespace {
<<__PHPStdLib>>
function ord(string $string)[]: int;
<<__PHPStdLib>>
function strip_tags(string $str, string $allowable_tags = ''): string;

<<__PHPStdLib>>
function intval($v, int $base = 10)[]: int;
<<__PHPStdLib>>
function doubleval($v)[]: float;
<<__PHPStdLib>>
function floatval($v)[]: float;
<<__PHPStdLib>>
function strval($v)[]: string;
<<__PHPStdLib>>
function boolval($v)[]: bool;

<<__PHPStdLib>>
function get_class_constants(string $class_name)[]: darray<string, mixed>;

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
  function ffp_parse_string(string $program)[]: ParseTree;

  function clear_static_memoization(?string $cls, ?string $func = null) : bool;
  function clear_lsb_memoization(string $cls, ?string $func = null) : bool;
  function clear_instance_memoization($obj) : bool;

  function is_list_like(mixed $arg)[]: bool;
  function is_class_meth(mixed $arg)[]: bool;
  function is_meth_caller(mixed $arg)[]: bool;
  function is_fun(mixed $arg)[]: bool;

  function set_frame_metadata(mixed $metadata)[]: void;

  // Code coverage
  function enable_per_file_coverage(keyset<string> $files): void;
  function disable_per_file_coverage(keyset<string> $files): void;
  function get_files_with_coverage(): keyset<string>;
  function get_coverage_for_file(string $file): vec<int>;
  function clear_coverage_for_file(string $file): void;
  function disable_all_coverage(): void;
  function get_all_coverage_data(): dict<string, vec<int>>;
  function clear_all_coverage_data(): void;

  function prefetch_units(keyset<string> $paths, bool $hint): void;
}
