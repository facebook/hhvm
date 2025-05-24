<?hh
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
use type HH\__Private\MiniTest\HackTest;

final class LowPriTest extends HackTest {

  public async function testBasicExecution(): Awaitable<void> {
    $expected_value = 42;
    $lowpri = new Async\LowPri();

    $result = await $lowpri->run(async () ==> {
      await SleepWaitHandle::create(10 * 1000); // 10ms
      return $expected_value;
    });

    expect($result)->toEqual($expected_value);
  }

  public async function testPrioritize(): Awaitable<void> {
    $expected_value = 84;
    $lowpri = new Async\LowPri();

    $lowpri->prioritize(); // Prioritize before running
    $result = await $lowpri->run(async () ==> {
      await SleepWaitHandle::create(10 * 1000); // 10ms
      return $expected_value;
    });

    expect($result)->toEqual($expected_value);
  }

  public async function testMultipleRuns(): Awaitable<void> {
    $expected_value = 42;
    $lowpri = new Async\LowPri();

    $task = $lowpri->run(async () ==> {
      await SleepWaitHandle::create(10 * 1000); // 10ms
      return $expected_value;
    });

    $result1 = await $task;
    expect($result1)->toEqual($expected_value);

    // Second run should fail with an invariant violation
    expect(async () ==> {
      await $lowpri->run(async () ==> 999);
    })->toThrow(\HH\InvariantException::class, 'Unable to run LowPri twice');
  }
}
