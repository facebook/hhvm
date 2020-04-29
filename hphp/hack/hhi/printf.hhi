<?hh   /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace {

/**
 * This type has some magic behavior: whenever it appears as a
 * function parameter (in a function with varargs), the argument must
 * be a static string, and will be parsed for % formatting specifiers
 * (which will determine the type of the varargs).
 *
 * T is treated as a state machine. After the first %, each character
 * causes the corresponding method in T to be looked up. For example,
 * '%b' will "call" the method
 *
 *   function format_b(int $s) : string;
 *
 * and consume an 'int' from the argument list.
 *
 * Hex escapes are used for non-alphabetic characters. The '%%'
 * pseudo-specifier consumes nothing and appears as
 *
 *     function format_0x25() : string;
 *
 * Modifiers and multi-char entries can be encoded by return a new
 * formatter instead of a string:
 *
 *     function format_upcase_l() : ListFormatter;
 *     function format_0x2a(int $s) : PaddingFormatter;
 *
 * Note that you *could* use an actual instance of T to do the
 * formatting. We don't; T is only here to provide the types.
 *
 * For another example on how to implement your own format string interface,
 * see \HH\Lib\Str\SprintfFormat in the HSL.
 */

interface PlainSprintf {
  // Technically %d should only take ints, but we don't.
 <<__Rx>>
  public function format_d(mixed $s) : string;
 <<__Rx>>
  public function format_s(?arraykey $s) : string;
 <<__Rx>>
  public function format_u(?int $s) : string;
 <<__Rx>>
  public function format_b(int $s) : string; // bit strings

  // Technically %f is locale-dependent (and thus wrong), but we don't.
 <<__Rx>>
  public function format_f(mixed $s) : string;
 <<__Rx>>
  public function format_g(?float $s) : string;
 <<__Rx>>
  public function format_upcase_f(?float $s) : string;
 <<__Rx>>
  public function format_upcase_e(?float $s) : string;
 <<__Rx>>
  public function format_e(?float $s) : string;

 <<__Rx>>

  public function format_x(mixed $s) : string;
 <<__Rx>>
  public function format_o(?int $s) : string;
 <<__Rx>>
  public function format_c(?int $s) : string;
 <<__Rx>>
  public function format_upcase_x(?int $s) : string;

  // %% takes no arguments
 <<__Rx>>
  public function format_0x25() : string;

  // Modifiers that don't change the type
 <<__Rx>>
  public function format_l() : PlainSprintf;
 <<__Rx>>
  public function format_0x20() : PlainSprintf; // ' '
 <<__Rx>>
  public function format_0x2b() : PlainSprintf; // '+'
 <<__Rx>>
  public function format_0x2d() : PlainSprintf; // '-'
 <<__Rx>>
  public function format_0x2e() : PlainSprintf; // '.'
 <<__Rx>>
  public function format_0x30() : PlainSprintf; // '0'
 <<__Rx>>
  public function format_0x31() : PlainSprintf; // ...
 <<__Rx>>
  public function format_0x32() : PlainSprintf;
 <<__Rx>>
  public function format_0x33() : PlainSprintf;
 <<__Rx>>
  public function format_0x34() : PlainSprintf;
 <<__Rx>>
  public function format_0x35() : PlainSprintf;
 <<__Rx>>
  public function format_0x36() : PlainSprintf;
 <<__Rx>>
  public function format_0x37() : PlainSprintf;
 <<__Rx>>
  public function format_0x38() : PlainSprintf;
 <<__Rx>>
  public function format_0x39() : PlainSprintf; // '9'
 <<__Rx>>
  public function format_0x27() : SprintfQuote;
}

// This should really be a wildcard. It's only used once.
interface SprintfQuote {
 <<__Rx>>
  public function format_0x3d() : PlainSprintf;
}

/**
 * sprintf uses PlainSprintf as its format string.
 * This type is very wide and will allow many incorrect calls to typecheck.
 * If possible, upgrade to \HH\Lib\Str\format().
 * This uses the far stricter \HH\Lib\Str\SprintfFormat type.
 */
<<__PHPStdLib, __Rx>>
function sprintf(\HH\FormatString<PlainSprintf> $fmt, ...$fmt_args): string;
/**
 * printf uses PlainSprintf as its format string.
 * This type is very wide and will allow many incorrect calls to typecheck.
 * If possible, upgrade to \HH\Lib\Str\format().
 * This uses the far stricter \HH\Lib\Str\SprintfFormat type.
 */
<<__PHPStdLib>>
function printf(\HH\FormatString<PlainSprintf> $fmt, ...$fmt_args): int;

}

namespace HH {

// Results in an \HH\InvariantException whose message is the result of
// calling sprintf with the arguments given this function.
// Equivalent to invariant(false, $fmt, ...$fmt_args).
<<__Rx>>
function invariant_violation(FormatString<\PlainSprintf> $fmt, ...$fmt_args): noreturn;

}
