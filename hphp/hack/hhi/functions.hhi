<?hh /* -*- php -*- */

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
namespace {
  //
  <<__PHPStdLib>>
  function array_fill<T>(
    int $start_index,
    int $num,
    T $value,
  )[]: varray_or_darray<T>;
  // TODO make non-nullable once Thrift files are fixed
  <<__PHPStdLib>>
  function chr(int $ascii)[]: string;
  <<__PHPStdLib>>
  function count(
    readonly mixed $x,
    int $mode = COUNT_NORMAL,
  )[]: int; // count takes Countable or array. We'll need to hardcode this...
  <<__PHPStdLib>>
  function dechex(int $number)[]: string;
  <<__PHPStdLib>>
  function implode(
    string $glue,
    readonly HH\FIXME\MISSING_PARAM_TYPE $pieces,
  )[]: string; // could be Container<Stringish>
  <<__PHPStdLib>>
  function explode(
    string $delimiter,
    string $str,
    int $limit = 0x7FFFFFFF,
  )[]: varray<string>; // : array<string> & false for '' delimiter
}

namespace HH {
  function is_vec(readonly mixed $arg)[]: bool;
  function is_dict(readonly mixed $arg)[]: bool;
  function is_keyset(readonly mixed $arg)[]: bool;

  /**
   * @returns True if `$arg` is a `varray`, `darray`, `dict`, `vec`, or `keyset`.
   * Otherwise returns false.
   */
  function is_any_array(readonly mixed $arg)[]: bool;
}

namespace {
  <<__PHPStdLib>>
  function ord(string $string)[]: int;
  <<__PHPStdLib>>
  function strip_tags(string $str, string $allowable_tags = ''): string;

  <<__PHPStdLib>>
  function intval(HH\FIXME\MISSING_PARAM_TYPE $v, int $base = 10)[]: int;
  <<__PHPStdLib>>
  function doubleval(HH\FIXME\MISSING_PARAM_TYPE $v)[]: float;
  <<__PHPStdLib>>
  function floatval(HH\FIXME\MISSING_PARAM_TYPE $v)[]: float;
  <<__PHPStdLib>>
  function strval(HH\FIXME\MISSING_PARAM_TYPE $v)[]: string;
  <<__PHPStdLib>>
  function boolval(HH\FIXME\MISSING_PARAM_TYPE $v)[]: bool;

  <<__PHPStdLib>>
  function get_class_constants(string $class_name)[]: darray<string, mixed>;
}

namespace HH {
  // autoload-map
  function could_include(\HH\FIXME\MISSING_PARAM_TYPE $file): bool;
  function autoload_is_native(): bool;

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
  function autoload_module_to_path(string $module): ?string;
  function autoload_type_alias_to_path(string $type_alias): ?string;
  function autoload_type_or_type_alias_to_path(string $type): ?string;

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
  function autoload_path_to_modules(string $path): vec<string>;
  function autoload_path_to_type_aliases(string $path): vec<string>;

  newtype ParseTree = darray<string, mixed>;
  function ffp_parse_string(string $program)[]: ParseTree;

  function clear_static_memoization(?string $cls, ?string $func = null): bool;
  function clear_lsb_memoization(string $cls, ?string $func = null): bool;
  function clear_instance_memoization(\HH\FIXME\MISSING_PARAM_TYPE $obj): bool;

  function is_list_like(readonly mixed $arg)[]: bool;
  function is_meth_caller(readonly mixed $arg)[]: bool;
  function is_fun(readonly mixed $arg)[]: bool;

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
