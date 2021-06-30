<?hh // decl
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH\Lib\_Private\_Str {

use type HH\Lib\_Private\_Locale\Locale;

function strlen_l(string $str, ?Locale $loc = null): int;
function uppercase_l(string $str, ?Locale $loc = null): string;
function lowercase_l(string $str, ?Locale $loc = null): string;
function titlecase_l(string $str, ?Locale $loc = null): string;
function foldcase_l(string $str, ?Locale $loc = null): string;
function chunk_l(string $str, int $chunk_size, ?Locale $loc = null): vec<string>;

function strcoll_l(string $a, string $b, ?Locale $loc = null): int;
function strcasecmp_l(string $a, string $b, ?Locale $loc = null): int;
function starts_with_l(string $str, string $prefix, ?Locale $loc = null): bool;
function starts_with_ci_l(string $str, string $prefix, ?Locale $loc = null): bool;
function ends_with_l(string $str, string $suffix, ?Locale $loc = null): bool;
function ends_with_ci_l(string $str, string $suffix, ?Locale $loc = null): bool;

function strpos_l(string $haystack, string $needle, int $offset, ?Locale $loc = null): int;
function strrpos_l(string $haystack, string $needle, int $offset, ?Locale $loc = null): int;
function stripos_l(string $haystack, string $needle, int $offset, ?Locale $loc = null): int;
function strripos_l(string $haystack, string $needle, int $offset, ?Locale $loc = null): int;

function slice_l(string $str, int $offset, int $length, ?Locale $loc = null): string;
function reverse_l(string $str, ?Locale $loc = null): string;
function vsprintf_l(?Locale $loc, string $fmt, vec<mixed> $args): string;

} // namespace HH\Lib\_Private\_Str
