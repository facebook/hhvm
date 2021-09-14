<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\HackTest;
use namespace HH\Lib\{Legacy_FIXME, Str};

final class StrLegacyFixmeTest extends HackTest {
  public function testNegativeSplitLimit(): void {
    expect(Legacy_FIXME\split_with_possibly_negative_limit('a!b!c', '!'))
      ->toEqual(vec['a', 'b', 'c']);
    expect(Legacy_FIXME\split_with_possibly_negative_limit('a!b!c', '!', 2))
      ->toEqual(vec['a', 'b!c']);
    expect(Legacy_FIXME\split_with_possibly_negative_limit('a!b!c', '!', -1))
      ->toEqual(vec['a', 'b']);
  }

  public function testReplacementCoercion(): void {
    expect(
      Str\replace_every(
        '1234',
        Legacy_FIXME\coerce_possibly_invalid_str_replace_pairs(
          /* HH_FIXME[4110] intentionally testing FIXMEd calls */
          Map {
            1 => 'One',
            2 => -2,
            3 => null,
          }
        )
      )
    )->toEqual('One-24');
  }

  public function testReplacementEmptyKeys(): void {
    expect(
      Str\replace_every(
        '123',
        Legacy_FIXME\coerce_possibly_invalid_str_replace_pairs(
          /* HH_FIXME[4110] intentionally testing FIXMEd calls */
          Map {
            '1' => 'One',
            '' => 'Two',
            '3' => 'Three',
          }
        )
      )
    )->toEqual('One2Three');
  }
}
