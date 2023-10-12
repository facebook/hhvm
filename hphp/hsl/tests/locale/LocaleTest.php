<?hh // strict
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

// Using `_Str` for now to test locales before they're exposed to `Str\`
use namespace HH\Lib\{Locale, _Private\_Str};
use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\HackTest;

// Intentionally using \sprintf() instead of Str\format() throughout as we want
// to test the request/thread locale, regardless of future changes to
// Str\format(), which we expect to change to always use 'C'
final class LocaleTest extends HackTest {
  public function __construct() {
    // Hopefully no-op...
    \setlocale(\LC_ALL, "C");
    // Prime the autoloader. HACKY HACK HACK. FIXME.
    //
    // As of 2021-08-17, the autoloader fails to find `ExpectObj` if the
    // locale is `fr_FR`. Let's pre-load.
    //
    // To reproduce the failure:
    // - change the `"C"` above to `"fr_FR"`
    // - `cd fbcode/hphp/hsl; ./minitest.sh LocaleTest`
    expect(123)->toEqual(123);
  }

  public function testBytesLocale(): void {
    \setlocale(\LC_ALL, "fr_FR");
    try {
      // Make sure the `\setlocale()` worked
      expect(\sprintf("%.02f", 1.23))->toEqual('1,23');

      $l = Locale\bytes();
      expect(_Str\strlen_l("💩", $l))->toEqual(4);
      expect(_Str\vsprintf_l($l, "%.02f", vec[1.23]))->toEqual('1.23');
    } finally {
      \setlocale(\LC_ALL, "C");
    }
  }

  public function testUTF8Locale(): void {
    \setlocale(\LC_ALL, "en_US.UTF-8");
    try {
      // Make sure the `\setlocale()` worked
      expect(\sprintf("%.02f", 1.23))->toEqual('1.23');

      $l = Locale\create('fr_FR.UTF-8');
      expect(_Str\strlen_l("💩", $l))->toEqual(1);
      expect(_Str\vsprintf_l($l, "%.02f", vec[1.23]))->toEqual('1,23');
    } finally {
      \setlocale(\LC_ALL, "C");
    }
  }

  public function testGetSet(): void {
    \setlocale(\LC_ALL, "fr_FR");
    try {
      expect(\setlocale(\LC_NUMERIC, '0'))->toEqual('fr_FR');
      $l = Locale\get_native();
      \setlocale(\LC_ALL, "C");
      expect(\setlocale(\LC_NUMERIC, '0'))->toEqual('C');
      expect(\sprintf('%.02f', 1.23))->toEqual('1.23');
      expect(_Str\vsprintf_l($l, '%.02f', vec[1.23]))->toEqual('1,23');
      Locale\set_native($l);
      expect(\sprintf('%.02f', 1.23))->toEqual('1,23');
      expect(\setlocale(\LC_NUMERIC, '0'))->toEqual('fr_FR');
    } finally {
      \setlocale(\LC_ALL, "C");
    }
  }

  public function testModified(): void {
    $c = Locale\bytes();
    expect(_Str\strlen_l('💩', $c))->toEqual(4);
    $c_utf8 = Locale\modified($c, Locale\Category::LC_CTYPE, 'en_US.UTF-8');
    expect(_Str\strlen_l('💩', $c_utf8))->toEqual(1);
    $fr_numeric = Locale\modified($c, Locale\Category::LC_NUMERIC, 'fr_FR.UTF-8');
    // as we didn't change LC_CTYPE, count bytes
    expect(_Str\strlen_l('💩', $fr_numeric))->toEqual(4);
    expect(_Str\vsprintf_l($fr_numeric, '%.02f', vec['1.23']))->toEqual('1,23');
  }

  public function testCBytesAlias(): void {
    // Poking around in internals, but guarantees all settings match
    /* HH_FIXME[3011] directly calling __debugInfo */
    $c_debug = Locale\c()->__debugInfo();
    expect($c_debug['LC_CTYPE'])->toEqual("C");
    /* HH_FIXME[3011] directly calling __debugInfo */
    expect($c_debug)->toEqual(Locale\bytes()->__debugInfo());
  }
}
