<?hh // strict
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\{Locale, OS, Str};
use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\{DataProvider, HackTest};

// This test only aims to make sure that the builtins are hooked up correctly;
// it does not aim to provide complete behavior coverage of the builtins, as
// that's already covered by the HHVM tests and the tests for the non-_l.
//
// Commonly used UTF-8 test sequences:
// - "\u{00e9}": "é" as a single character
// - "e\u{0301}": "é" as two characters, 'e', and a combining acute accent
// - "\u{00c9}": "É" as a single character
// - "E\u{0301}": "É" as two characters, 'E', and a combining acute accent
final class StrLocaleTest extends HackTest {
  public function testSystemDefaultEncoding(): void {
    // This is undefined behavior, but let's make sure that we act like
    // everything else on the current platform
    $emoji = "😀☹️";
    $actual = Str\slice_l(Locale\create('en_US'), $emoji, 0, 1);
    if (\HH\Lib\_Private\_OS\IS_MACOS) {
      // UTF-8
      expect($actual)->toEqual("😀");
    } else {
      // Single-byte (e.g. ASCII or ISO8859-1)
      expect($actual)->toEqual("😀"[0]);
    }
  }

  public function testCapitalizeL(): void {
    expect(Str\capitalize_l(Locale\create('en_US'), 'ifoo'))
      ->toEqual('Ifoo');
    expect(Str\capitalize_l(Locale\create('en_US.UTF-8'), 'ifoo'))
      ->toEqual('Ifoo');
    expect(Str\capitalize_l(Locale\create('tr_TR.UTF-8'), 'ifoo'))
      ->toEqual('İfoo');
  }

  public function testCapitalizeWordsL(): void {
    expect(Str\capitalize_words_l(Locale\create('en_US'), 'ifoo ibar'))
      ->toEqual('Ifoo Ibar');
    expect(Str\capitalize_words_l(Locale\create('en_US.UTF-8'), 'ifoo ibar'))
      ->toEqual('Ifoo Ibar');
    expect(Str\capitalize_words_l(Locale\create('tr_TR.UTF-8'), 'ifoo ibar'))
      ->toEqual('İfoo İbar');
  }

  public function testChunkL(): void {
    $emoji = "😀😀😀😀";
    expect(Str\chunk_l(Locale\c(), $emoji, 2))
      ->toEqual(\str_split($emoji, 2));
    expect(Str\chunk_l(Locale\create('en_US.ISO8859-1'), $emoji, 2))
      ->toEqual(\str_split($emoji, 2));
    expect(Str\chunk_l(Locale\create('en_US.UTF-8'), $emoji, 2))
      ->toEqual(vec["😀😀", "😀😀"]);
  }

  public function testFormatL(): void {
    expect(Str\format_l(Locale\create('en_US'), '%.2f', 1.23))
      ->toEqual('1.23');
    expect(Str\format_l(Locale\create('fr_FR'), '%.2f', 1.23))
      ->toEqual('1,23');
  }

  public function testCompareL(): void {
    $a = 'Apple';
    $b = 'Æble';
    expect($b)->toEqual("\u{00c6}ble");
    expect($b)->toEqual("\xc3\x86ble");
    $b_iso8859_1 = "\xc6ble";

    expect(Str\compare_l(Locale\c(), $a, $b))->toBeLessThan(0);
    expect(Str\compare_l(Locale\create('en_US.ISO8859-1'), $a, $b))->toBeGreaterThan(0);
    expect(Str\compare_l(Locale\create('en_US.UTF-8'), $a, $b))->toBeGreaterThan(0);
    // Danish has different sorting rules...
    expect(Str\compare_l(Locale\create('da_DK.UTF-8'), $a, $b))->toBeLessThan(0);
    // ... but if we don't specify UTF8, we have different behavior:
    //
    // In $b (UTF-8), 'Æ' is 0xc386, but da_DK is usually
    // ISO-8859-1 on Linux, which considers that to be two characters.
    expect(Str\compare_l(Locale\create('da_DK.ISO8859-1'), $a, $b))->toBeGreaterThan(0);
  }

  public function testCompareCIL(): void {
    $a = 'Apple';
    $b = 'æble';
    expect(Str\compare_ci_l(Locale\c(), $a, $b))->toBeLessThan(0);
    expect(Str\compare_ci_l(Locale\create('en_US.UTF-8'), $a, $b))->toBeGreaterThan(0);
    expect(Str\compare_l(Locale\create('da_DK.UTF-8'), $a, $b))->toBeLessThan(0);
  }

  public function testContainsL(): void {
    $e_combined_acute = "e\u{0301}";
    $e_acute = "\u{00e9}";
    $emoji = "😀";

    $l = Locale\c();
    expect(Str\contains_l($l, $e_combined_acute, 'e'))->toBeTrue();
    expect(Str\contains_l($l, $e_acute, 'e'))->toBeFalse();
    expect(Str\contains_l($l, $e_combined_acute, $e_acute))->toBeFalse();
    expect(Str\contains_l($l, $e_acute, $e_combined_acute))->toBeFalse();
    expect(Str\contains_l($l, $emoji, $emoji))->toBeTrue();
    expect(Str\contains_l($l, $emoji, $emoji[0]))->toBeTrue();

    $l = Locale\create('en_US.ISO8859-1');
    expect(Str\contains_l($l, $e_combined_acute, 'e'))->toBeTrue();
    expect(Str\contains_l($l, $e_acute, 'e'))->toBeFalse();
    expect(Str\contains_l($l, $e_combined_acute, $e_acute))->toBeFalse();
    expect(Str\contains_l($l, $e_acute, $e_combined_acute))->toBeFalse();
    expect(Str\contains_l($l, $emoji, $emoji))->toBeTrue();
    expect(Str\contains_l($l, $emoji, $emoji[0]))->toBeTrue();

    $l = Locale\create('en_US.UTF-8');
    expect(Str\contains_l($l, $e_combined_acute, 'e'))->toBeFalse();
    expect(Str\contains_l($l, $e_acute, 'e'))->toBeFalse();
    expect(Str\contains_l($l, $e_combined_acute, $e_acute))->toBeTrue();
    expect(Str\contains_l($l, $e_acute, $e_combined_acute))->toBeTrue();
    expect(Str\contains_l($l, $emoji, $emoji))->toBeTrue();
    expect(Str\contains_l($l, $emoji, $emoji[0]))->toBeFalse();
  }

  public function testContainsCIL(): void {
    $e_combined_acute = "E\u{0301}";
    $e_acute = "\u{00e9}";
    $emoji = "😀";

    $l = Locale\c();
    expect(Str\contains_ci_l($l, $e_combined_acute, 'e'))->toBeTrue();
    expect(Str\contains_ci_l($l, $e_acute, 'e'))->toBeFalse();
    expect(Str\contains_ci_l($l, $e_combined_acute, $e_acute))->toBeFalse();
    expect(Str\contains_ci_l($l, $e_acute, $e_combined_acute))->toBeFalse();
    expect(Str\contains_ci_l($l, $emoji, $emoji))->toBeTrue();
    expect(Str\contains_ci_l($l, $emoji, $emoji[0]))->toBeTrue();

    $l = Locale\create('en_US.ISO8859-1');
    expect(Str\contains_ci_l($l, $e_combined_acute, 'e'))->toBeTrue();
    expect(Str\contains_ci_l($l, $e_acute, 'e'))->toBeFalse();
    expect(Str\contains_ci_l($l, $e_combined_acute, $e_acute))->toBeFalse();
    expect(Str\contains_ci_l($l, $e_acute, $e_combined_acute))->toBeFalse();
    expect(Str\contains_ci_l($l, $emoji, $emoji))->toBeTrue();
    expect(Str\contains_ci_l($l, $emoji, $emoji[0]))->toBeTrue();

    $l = Locale\create('en_US.UTF-8');
    expect(Str\contains_ci_l($l, $e_combined_acute, 'e'))->toBeFalse();
    expect(Str\contains_ci_l($l, $e_acute, 'e'))->toBeFalse();
    expect(Str\contains_ci_l($l, $e_combined_acute, $e_acute))->toBeTrue();
    expect(Str\contains_ci_l($l, $e_acute, $e_combined_acute))->toBeTrue();
    expect(Str\contains_ci_l($l, $emoji, $emoji))->toBeTrue();
    expect(Str\contains_ci_l($l, $emoji, $emoji[0]))->toBeFalse();
  }

  public function testEndsWithL(): void {
    $def = "de\u{0301}f";

    $l = Locale\create('en_US.ISO8859-1');
    expect(Str\ends_with_l($l, $def, "e\u{0301}f"))->toBeTrue();
    expect(Str\ends_with_l($l, $def, "\u{00e9}f"))->toBeFalse();
    expect(Str\ends_with_l($l, $def, "E\u{0301}f"))->toBeFalse();

    $l = Locale\create('en_US.UTF-8');
    expect(Str\ends_with_l($l, $def, "e\u{0301}f"))->toBeTrue();
    expect(Str\ends_with_l($l, $def, "\u{00e9}f"))->toBeTrue();
    expect(Str\ends_with_l($l, $def, "E\u{0301}f"))->toBeFalse();
  }

  public function testEndsWithCIL(): void {
    $def = "de\u{0301}f";

    $l = Locale\create('en_US.ISO8859-1');
    expect(Str\ends_with_ci_l($l, $def, "E\u{0301}f"))->toBeTrue();
    expect(Str\ends_with_ci_l($l, $def, "\u{00c9}f"))->toBeFalse();

    $l = Locale\create('en_US.UTF-8');
    expect(Str\ends_with_ci_l($l, $def, "E\u{0301}f"))->toBeTrue();
    expect(Str\ends_with_ci_l($l, $def, "\u{00c9}f"))->toBeTrue();
  }

  public function testLengthL(): void {
    $emoji = "😀😀😀😀";
    expect(Str\length_l(Locale\create('en_US.UTF-8'), $emoji))->toEqual(4);
    expect(Str\length_l(Locale\create('en_US.ISO8859-1'), $emoji))->toEqual(16);
  }

  public function testLowercaseL(): void {
    expect(Str\lowercase_l(Locale\c(), "IFOO"))->toEqual('ifoo');
    expect(Str\lowercase_l(Locale\create('en_US.ISO8859-1'), "IFOO"))->toEqual('ifoo');
    expect(Str\lowercase_l(Locale\create('en_US.UTF-8'), "IFOO"))->toEqual('ifoo');
    expect(Str\lowercase_l(Locale\create('tr_TR.UTF-8'), "IFOO"))->toEqual('ıfoo');
  }

  public function testPadLeftL(): void {
    $emoji = "😀😀😀😀";
    expect(Str\pad_left_l(Locale\c(), $emoji, 5, '!'))->toEqual($emoji);
    expect(Str\pad_left_l(Locale\create('en_US.ISO8859-1'), $emoji, 5, '!'))->toEqual($emoji);
    expect(Str\pad_left_l(Locale\create('en_US.UTF-8'), $emoji, 5, '!'))->toEqual('!'.$emoji);
    expect(Str\pad_left_l(Locale\create('en_US.UTF-8'), $emoji, 6, '!'))->toEqual('!!'.$emoji);
    expect(Str\pad_left_l(Locale\create('en_US.UTF-8'), $emoji, 6, '😀'))->toEqual('😀😀'.$emoji);
  }

  public function testPadRightL(): void {
    $emoji = "😀😀😀😀";
    expect(Str\pad_right_l(Locale\c(), $emoji, 5, '!'))->toEqual($emoji);
    expect(Str\pad_right_l(Locale\create('en_US.ISO8859-1'), $emoji, 5, '!'))->toEqual($emoji);
    expect(Str\pad_right_l(Locale\create('en_US.UTF-8'), $emoji, 5, '!'))->toEqual($emoji.'!');
    expect(Str\pad_right_l(Locale\create('en_US.UTF-8'), $emoji, 6, '!'))->toEqual($emoji.'!!');
    expect(Str\pad_right_l(Locale\create('en_US.UTF-8'), $emoji, 6, '😀'))->toEqual($emoji.'😀😀');
  }

  public function testReplaceL(): void {
    $def = "de\u{0301}f";

    $l = Locale\create('en_US.ISO8859-1');
    expect(Str\replace_l($l, $def, 'e', 'E'))->toEqual("dE\u{0301}f");
    expect(Str\replace_l($l, $def, "\u{00e9}", '!'))->toEqual($def);
    expect(Str\replace_l($l, $def, "\u{00c9}", '!'))->toEqual($def);

    $l = Locale\create("en_US.UTF-8");
    expect(Str\replace_l($l, $def, 'e', 'E'))->toEqual("d\u{00e9}f");
    expect(Str\replace_l($l, $def, "\u{00e9}", '!'))->toEqual('d!f');
    expect(Str\replace_l($l, $def, "\u{00c9}", '!'))->toEqual("d\u{00e9}f");
  }

  public function testReplaceCIL(): void {
    $def = "de\u{0301}f";

    $l = Locale\create('en_US.ISO8859-1');
    expect(Str\replace_ci_l($l, $def, 'e', 'E'))->toEqual("dE\u{0301}f");
    expect(Str\replace_ci_l($l, $def, "\u{00e9}", '!'))->toEqual($def);
    expect(Str\replace_ci_l($l, $def, "\u{00c9}", '!'))->toEqual($def);

    $l = Locale\create("en_US.UTF-8");
    expect(Str\replace_ci_l($l, $def, 'e', 'E'))->toEqual("d\u{00e9}f");
    expect(Str\replace_ci_l($l, $def, "\u{00e9}", '!'))->toEqual('d!f');
    expect(Str\replace_ci_l($l, $def, "\u{00c9}", '!'))->toEqual('d!f');
    expect(Str\replace_ci_l($l, 'hij', "I", '!'))->toEqual('h!j');
    expect(Str\replace_ci_l($l, 'hij', "İ", '!'))->toEqual('hij');

    $l = Locale\create("tr_TR.UTF-8");
    expect(Str\replace_ci_l($l, 'hij', "I", '!'))->toEqual('hij');
    expect(Str\replace_ci_l($l, 'hij', "İ", '!'))->toEqual('h!j');
  }

  public function testReverseL(): void {
    $emoji = '😀💩';
    $ijome = '💩😀';

    expect(Str\reverse_l(Locale\create('en_US.ISO8859-1'), $emoji))
      ->toNotEqual($ijome);
    expect(Str\reverse_l(Locale\create('en_US.ISO8859-1'), $emoji))
      ->toEqual(\strrev($emoji));

    expect(Str\reverse_l(Locale\create('en_US.UTF-8'), $emoji))
      ->toEqual($ijome);
  }

  public function testSearchL(): void {
    $def = "de\u{0301}f";

    $l = Locale\create('en_US.ISO8859-1');
    expect(Str\search_l($l, $def, 'e'))->toEqual(1);
    expect(Str\search_l($l, $def, 'f'))->toEqual(4);
    expect(Str\search_l($l, $def, 'F'))->toEqual(null);

    $l = Locale\create('en_US.UTF-8');
    expect(Str\search_l($l, $def, 'e'))->toEqual(null);
    expect(Str\search_l($l, $def, 'f'))->toEqual(2);
    expect(Str\search_l($l, $def, 'F'))->toEqual(null);
  }

  public function testSearchCIL(): void {
    $def = "de\u{0301}f";

    $l = Locale\create('en_US.ISO8859-1');
    expect(Str\search_ci_l($l, $def, 'e'))->toEqual(1);
    expect(Str\search_ci_l($l, $def, 'f'))->toEqual(4);
    expect(Str\search_ci_l($l, $def, 'F'))->toEqual(4);

    $l = Locale\create('en_US.UTF-8');
    expect(Str\search_ci_l($l, $def, 'e'))->toEqual(null);
    expect(Str\search_ci_l($l, $def, 'f'))->toEqual(2);
    expect(Str\search_ci_l($l, $def, 'F'))->toEqual(2);
  }

  public function testSearchLastL(): void {
    $def = "d\u{00e9}fde\u{0301}f";

    $l = Locale\create('en_US.ISO8859-1');
    expect(Str\search_last_l($l, $def, 'e'))->toEqual(5);
    expect(Str\search_last_l($l, $def, "\u{00e9}"))->toEqual(1);
    expect(Str\search_last_l($l, $def, "E"))->toEqual(null);
    expect(Str\search_last_l($l, $def, "\u{00c9}"))->toEqual(null);

    $l = Locale\create('en_US.UTF-8');
    expect(Str\search_last_l($l, $def, 'e'))->toEqual(null);
    expect(Str\search_last_l($l, $def, "\u{00e9}"))->toEqual(4);
    expect(Str\search_last_l($l, $def, "E"))->toEqual(null);
    expect(Str\search_last_l($l, $def, "\u{00c9}"))->toEqual(null);
  }

  public function testSliceL(): void {
    $emoji = "😀😀😀😀";
    expect(Str\slice_l(Locale\create('en_US.ISO8859-1'), $emoji, 0, 2))
      ->toEqual(\substr($emoji, 0, 2));
    expect(Str\slice_l(Locale\create('en_US.UTF-8'), $emoji, 0, 2))
      ->toNotEqual(\substr($emoji, 0, 2));
    expect(Str\slice_l(Locale\create('en_US.UTF-8'), $emoji, 0, 2))
      ->toEqual("😀😀");
  }

  public function testSpliceL(): void {
    $def = "de\u{0301}f";
    expect(Str\splice_l(Locale\create('en_US.ISO8859-1'), $def, 'E', 1, 1))
      ->toEqual("dE\u{0301}f");
    expect(Str\splice_l(Locale\create('en_US.UTF-8'), $def, 'E', 1, 1))
      ->toEqual("dEf");
  }

  public function testSplitL(): void {
    $def = "de\u{0301}f";
    expect(Str\split_l(Locale\c(), $def, "e"))
      ->toEqual(vec['d', "\u{0301}f"]);
    // _l functions operate on normalized forms, so even something that seems
    // like a no-op might not be.
    expect(Str\split_l(Locale\create('en_US.UTF-8'), $def, "e"))
      ->toEqual(vec["d\u{00e9}f"]);

    expect(Str\split_l(Locale\c(), $def, "\xcc"))
      ->toEqual(vec['de', "\x81f"]);
    expect(Str\split_l(Locale\create("en_US.UTF-8"), $def, "\xcc"))
      ->toEqual(vec["d\u{00e9}f"]);

    expect(Str\split_l(Locale\c(), $def, "\u{00e9}"))
      ->toEqual(vec[$def]);
    expect(Str\split_l(Locale\create("en_US.UTF-8"), $def, "\u{00e9}"))
      ->toEqual(vec['d', 'f']);
  }

  public function testStartsWithL(): void {
    $def = "de\u{0301}f";

    $l = Locale\create('en_US.ISO8859-1');
    expect(Str\starts_with_l($l, $def, "de\u{0301}"))->toBeTrue();
    expect(Str\starts_with_l($l, $def, "d\u{00e9}"))->toBeFalse();
    expect(Str\starts_with_l($l, $def, "dE\u{0301}"))->toBeFalse();

    $l = Locale\create('en_US.UTF-8');
    expect(Str\starts_with_l($l, $def, "de\u{0301}"))->toBeTrue();
    expect(Str\starts_with_l($l, $def, "d\u{00e9}"))->toBeTrue();
    expect(Str\starts_with_l($l, $def, "dE\u{0301}"))->toBeFalse();
  }

  public function testStartsWithCIL(): void {
    $def = "de\u{0301}f";

    $l = Locale\create('en_US.ISO8859-1');
    expect(Str\starts_with_ci_l($l, $def, "de\u{0301}"))->toBeTrue();
    expect(Str\starts_with_ci_l($l, $def, "d\u{00e9}"))->toBeFalse();
    expect(Str\starts_with_ci_l($l, $def, "dE\u{0301}"))->toBeTrue();

    $l = Locale\create('en_US.UTF-8');
    expect(Str\starts_with_ci_l($l, $def, "de\u{0301}"))->toBeTrue();
    expect(Str\starts_with_ci_l($l, $def, "d\u{00e9}"))->toBeTrue();
    expect(Str\starts_with_ci_l($l, $def, "dE\u{0301}"))->toBeTrue();
  }

  public function testStripPrefixL(): void {
    $def = "de\u{0301}f";

    expect(Str\strip_prefix_l(Locale\create('en_US.ISO8859-1'), $def, "d\u{00e9}"))
      ->toEqual($def);
    expect(Str\strip_prefix_l(Locale\create('en_US.UTF-8'), $def, "d\u{00e9}"))
      ->toEqual('f');
  }

  public function testStripSuffixL(): void {
    $def = "de\u{0301}f";

    expect(Str\strip_suffix_l(Locale\create('en_US.ISO8859-1'), $def, "\u{00e9}f"))
      ->toEqual($def);
    expect(Str\strip_suffix_l(Locale\create('en_US.UTF-8'), $def, "\u{00e9}f"))
      ->toEqual('d');
  }

  public function testTrimL(): void {
    // \u{00a0} is unicode non-breaking space
    $sample = " \u{00a0}!\u{00a0} ";
    expect(Str\trim_l(Locale\create('en_US.ISO8859-1'), $sample))
      ->toEqual("\u{00a0}!\u{00a0}");
    expect(Str\trim_l(Locale\create('en_US.UTF-8'), $sample))
      ->toEqual("!");
  }

  public function testTrimLeftL(): void {
    // 00a0 == nbsp
    $sample = " \u{00a0}!\u{00a0} ";
    expect(Str\trim_left_l(Locale\create('en_US.ISO8859-1'), $sample))
      ->toEqual("\u{00a0}!\u{00a0} ");
    expect(Str\trim_left_l(Locale\create('en_US.UTF-8'), $sample))
      ->toEqual("!\u{00a0} ");
  }

  public function testTrimRightL(): void {
    // 00a0 == nbsp
    $sample = " \u{00a0}!\u{00a0} ";
    expect(Str\trim_right_l(Locale\create('en_US.ISO8859-1'), $sample))
      ->toEqual(" \u{00a0}!\u{00a0}");
    expect(Str\trim_right_l(Locale\create('en_US.UTF-8'), $sample))
      ->toEqual(" \u{00a0}!");
  }

  public function testUppercaseL(): void {
    expect(Str\uppercase_l(Locale\create('en_US.UTF-8'), 'ifoo'))->toEqual('IFOO');
    expect(Str\uppercase_l(Locale\create('tr_TR.UTF-8'), 'ifoo'))->toEqual('İFOO');
  }
}
