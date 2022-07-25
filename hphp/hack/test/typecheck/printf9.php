<?hh
/**
 * (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

interface X {
  public function format_0x3d(): Y;
  public function format_0x23(): X;
  public function format_s(string $s): string;
  public function format_upcase_n(float $f): string;
}

interface Y {
  public function format_0x25(): Y;
  public function format_upcase_n(int $i): string;
  public function format_wild(): X;
}

function f(HH\FormatString<PlainSprintf> $f, mixed ...$_): void {
}

function g(HH\FormatString<X> $f, mixed ...$_): void {
}

function _(): void {
  f('+%\'X-11s', 'hello');
  f("%+.0.'& .5-f", 3.14);  // nonsense, but typechecks
  g('x %=us %=N. %=R=m#=Z##s', 'foo', 42, 'bar');
  g('%=s=N %=%%%.s %=ss%N', -5, 'baz', 'qux', 6.);
  g('%=s'); // error
}
