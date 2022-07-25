<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\Str;

use namespace HH\Lib\_Private\_Str;

/**
 * This interface describes features of a valid format string for `Str\format`
 */
interface SprintfFormat {
  public function format_d(int $s): string;
  public function format_s(string $s): string;
  public function format_u(int $s): string;
  public function format_b(int $s): string; // bit strings
  // Technically %f is locale-dependent (and thus wrong)
  public function format_f(float $s): string;
  public function format_g(float $s): string;
  public function format_upcase_f(float $s): string;
  public function format_e(float $s): string;
  public function format_upcase_e(float $s): string;
  public function format_x(int $s): string;
  public function format_o(int $s): string;
  public function format_c(int $s): string;
  public function format_upcase_x(int $s): string;
  // %% takes no arguments
  public function format_0x25(): string;
  // %'(char)
  public function format_0x27(): SprintfFormatQuote;
  // Modifiers that don't change the type
  public function format_l(): SprintfFormat;
  public function format_0x20(): SprintfFormat; // ' '
  public function format_0x2b(): SprintfFormat; // '+'
  public function format_0x2d(): SprintfFormat; // '-'
  public function format_0x2e(): SprintfFormat; // '.'
  public function format_0x30(): SprintfFormat; // '0'
  public function format_0x31(): SprintfFormat; // ...
  public function format_0x32(): SprintfFormat;
  public function format_0x33(): SprintfFormat;
  public function format_0x34(): SprintfFormat;
  public function format_0x35(): SprintfFormat;
  public function format_0x36(): SprintfFormat;
  public function format_0x37(): SprintfFormat;
  public function format_0x38(): SprintfFormat;
  public function format_0x39(): SprintfFormat; // '9'
}

/**
 * Accessory interface for `SprintfFormat`
 *
 * @guide /hack/built-in-types/string
 * @guide /hack/functions/format-strings
 */
interface SprintfFormatQuote {
  public function format_wild(): SprintfFormat;
}

type SprintfFormatString = \HH\FormatString<SprintfFormat>;

/**
 * Given a valid format string (defined by `SprintfFormatString`), return a
 * formatted string using `$format_args`
 *
 * @guide /hack/built-in-types/string
 * @guide /hack/functions/format-strings
 */
function format(
  SprintfFormatString $format_string,
  mixed ...$format_args
)[]: string {
  return _Str\vsprintf_l(null, $format_string as string, $format_args);
}
