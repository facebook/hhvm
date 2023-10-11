<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\Str;
use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\{DataProvider, HackTest};

final class StrFormatTest extends HackTest {

  public static function provideFormat(): varray<mixed> {
    return varray[
      tuple(Str\format('No format specifiers'), 'No format specifiers'),
      tuple(
        Str\format('A single %s', 'string specifier'),
        'A single string specifier',
      ),
      tuple(
        Str\format("Width modifiers: %5s %'=5s %'X-6s", 'abc', 'abc', 'abc'),
        'Width modifiers:   abc ==abc abcXXX',
      ),
      tuple(
        Str\format(
          'Number specifiers: %d %.3f %.2e %.2E',
          42,
          3.14159,
          1200.,
          1200.,
        ),
        'Number specifiers: 42 3.142 1.20e+3 1.20E+3',
      ),
      tuple(
        Str\format('Base specifiers: %b %o %x %X', 15, 15, 15, 15),
        'Base specifiers: 1111 17 f F',
      ),
      tuple(Str\format('Percent specifier: %%'), 'Percent specifier: %'),
    ];
  }

  <<DataProvider('provideFormat')>>
  public function testFormat(string $actual, string $expected): void {
    try {
      \setlocale(\LC_ALL, 'fr_FR');
      expect($actual)->toEqual($expected);
    } finally {
      \setlocale(\LC_ALL, 'C');
    }
  }
}
