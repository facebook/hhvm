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
  public function format_0x27(): SprintfFormatQuote;
}

/**
 * Accessory interface for `SprintfFormat`
 * Note: This should really be a wildcard. It's only used once (with '=').
 */
interface SprintfFormatQuote {
  public function format_0x3d(): SprintfFormat;
}

type SprintfFormatString = \HH\FormatString<SprintfFormat>;

/**
 * Given a valid format string (defined by `SprintfFormatString`), return a
 * formatted string using `$format_args`
 */
function format(
  SprintfFormatString $format_string,
  mixed ...$format_args
)[]: string {
  return \vsprintf($format_string, $format_args);
}
