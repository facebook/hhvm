<?hh // strict
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\Async;

use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\{DataProvider, HackTest};

final class PollTest extends HackTest {
  public async function testEmpty(): Awaitable<void> {
    foreach (Async\Poll::create() await as $value) {
      throw new Exception('expected to be empty');
    }
  }

  public async function testEmptyKeyed(): Awaitable<void> {
    foreach (Async\KeyedPoll::create() await as $value) {
      throw new Exception('expected to be empty');
    }
  }

  public static function provideTestInputs(
  ): varray<(varray<Awaitable<int>>, varray<int>, bool)> {
    $block = async (int $prio) ==> await RescheduleWaitHandle::create(0, $prio);

    return varray[
      // Empty
      tuple(
        varray[],
        varray[],
        false,
      ),
      // One eager success
      tuple(
        varray[async { return 42; }],
        varray[42],
        false,
      ),
      // One resumed success
      tuple(
        varray[async { await $block(0); return 42; }],
        varray[42],
        false,
      ),
      // One eager failure
      tuple(
        varray[async { throw new Exception(); }],
        varray[],
        true,
      ),
      // One resumed failure
      tuple(
        varray[async { await $block(0); throw new Exception(); }],
        varray[],
        true,
      ),
      // Combination
      tuple(
        varray[
          async { return 1; },
          async { await $block(1); return 2; },
          async { return 3; },
          async { await $block(3); return 4; },
          async { await $block(2); throw new Exception(); },
          async { await $block(0); return 5; },
          async { return 6; },
        ],
        varray[1, 3, 6, 5, 2],
        true,
      ),
    ];
  }

  <<DataProvider('provideTestInputs')>>
  public async function testInputs(
    varray<Awaitable<int>> $awaitables,
    varray<int> $expected_results,
    bool $expected_exception,
  ): Awaitable<void> {
    $results = varray[];
    $exception = false;
    try {
      foreach (Async\Poll::from($awaitables) await as $value) {
        $results[] = $value;
      }
    } catch (Exception $e) {
      if (!$expected_exception) {
        throw $e;
      }
      $exception = true;
    }

    expect($results)->toEqual($expected_results);
    expect($exception)->toEqual($expected_exception);
  }

  public async function testKeyed(): Awaitable<void> {
    $poll = Async\KeyedPoll::from(darray[
      'k1' => async {
        return 1;
      },
    ]);
    $poll->add('k2', async { return 2; });
    $count = 0;
    foreach($poll await as $key => $value) {
      $count += 1;
      switch ($key) {
        case 'k1':
          expect($value)->toBePHPEqual(1);
          break;
        case 'k2':
          expect($value)->toBePHPEqual(2);
          $poll->addMulti(darray['k3' => async { return 3; }]);
          break;
        case 'k3':
          expect($value)->toBePHPEqual(3);
          break;
        default:
          invariant_violation("unexpected key: %s", $key);
      }
    }
    expect($count)->toBePHPEqual(3);
  }
}
