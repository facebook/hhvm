<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\Legacy_FIXME;
use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\HackTest;

final class IncDecCoercionTest extends HackTest {
  public function testIncrement(): void {
    expect(Legacy_FIXME\increment(null))->toEqual(1);
    expect(Legacy_FIXME\increment('1234'))->toEqual(1235);
    expect(Legacy_FIXME\increment('a12'))->toEqual('a13');
    expect(Legacy_FIXME\increment(''))->toEqual('1');
    expect(Legacy_FIXME\increment(42))->toEqual(43);
  }

  public function testDecrement(): void {
    expect(Legacy_FIXME\decrement(null))->toEqual(null);
    expect(Legacy_FIXME\decrement('1234'))->toEqual(1233);
    expect(Legacy_FIXME\decrement('a12'))->toEqual('a12');
    expect(Legacy_FIXME\decrement(''))->toEqual(-1);
    expect(Legacy_FIXME\decrement(42))->toEqual(41);
  }
}
