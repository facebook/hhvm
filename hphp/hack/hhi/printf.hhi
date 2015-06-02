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
 * This type has some magic behavior: whenever it appears as a
 * function parameter (in a function with varargs), the argument must
 * be a static string, and will be parsed for % formatting specifiers
 * (which will determine the type of the varargs).
 *
 * T is treated as a state machine. After the first %, each character
 * causes the corresponding method in T to be looked up. For example,
 * '%d' will "call" the method
 *
 *   function format_d(?int $s) : string;
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
 */

interface PlainSprintf {
  // It's common to pass floats; would be nice to type this as
  // 'number' once that type becomes available in userland.
  public function format_d(mixed $s) : string;
  public function format_s(mixed $s) : string;
  public function format_u(?int $s) : string;
  public function format_b(int $s) : string; // bit strings

  // Technically %f is locale-dependent (and thus wrong), but we don't.
  public function format_f(mixed $s) : string;
  public function format_g(?float $s) : string;
  public function format_upcase_f(?float $s) : string;
  public function format_upcase_e(?float $s) : string;

  public function format_x(mixed $s) : string;
  public function format_o(?int $s) : string;
  public function format_c(?int $s) : string;
  public function format_upcase_x(?int $s) : string;

  // %% takes no arguments
  public function format_0x25() : string;

  // Modifiers that don't change the type
  public function format_l() : PlainSprintf;
  public function format_0x20() : PlainSprintf; // ' '
  public function format_0x2b() : PlainSprintf; // '+'
  public function format_0x2d() : PlainSprintf; // '-'
  public function format_0x2e() : PlainSprintf; // '.'
  public function format_0x30() : PlainSprintf; // '0'
  public function format_0x31() : PlainSprintf; // ...
  public function format_0x32() : PlainSprintf;
  public function format_0x33() : PlainSprintf;
  public function format_0x34() : PlainSprintf;
  public function format_0x35() : PlainSprintf;
  public function format_0x36() : PlainSprintf;
  public function format_0x37() : PlainSprintf;
  public function format_0x38() : PlainSprintf;
  public function format_0x39() : PlainSprintf; // '9'
  public function format_0x27() : SprintfQuote;
}

// This should really be a wildcard. It's only used once.
interface SprintfQuote {
  public function format_0x3d() : PlainSprintf;
}

function sprintf(\HH\FormatString<PlainSprintf> $fmt, ...$fmt_args): string;
function printf(\HH\FormatString<PlainSprintf> $fmt, ...$fmt_args): void;

// Results in an \HH\InvariantException whose message is the result of
// calling sprintf with the arguments given this function
// function invariant_violation(\HH\FormatString<PlainSprintf> $fmt, ...$fmt_args): noreturn;
function invariant_violation(string $fmt, ...$fmt_args): noreturn;
