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

} // namespace HH\Lib\_Private\_Str
