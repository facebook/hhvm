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
use function HH\Lib\_Private\stop_eager_execution;

final class ConditionTest extends HackTest {

  public async function testSucceedFirst(): Awaitable<void> {
    $condition = new Async\Condition();
    $condition->succeed(42);
    $result = await $condition->waitForNotificationAsync(async {
    });
    expect($result)->toBePHPEqual(42);
  }

  public async function testFailFirst(): Awaitable<void> {
    $condition = new Async\Condition();
    $condition->fail(new Exception('hello world'));
    try {
      $result = await $condition->waitForNotificationAsync(async {
      });
      throw new Exception('expected to fail');
    } catch (Exception $e) {
      expect($e->getMessage())->toBePHPEqual('hello world');
    }
  }

  public async function testSucceedLater(): Awaitable<void> {
    $condition = new Async\Condition();
    $worker = async {
      await stop_eager_execution();
      $condition->succeed(42);
    };

    $result = await $condition->waitForNotificationAsync($worker);
    expect($result)->toBePHPEqual(42);
  }

  public async function testFailLater(): Awaitable<void> {
    $condition = new Async\Condition();
    $worker = async {
      await stop_eager_execution();
      $condition->fail(new Exception('hello world'));
    };

    try {
      $result = await $condition->waitForNotificationAsync($worker);
      throw new Exception('expected to fail');
    } catch (Exception $e) {
      expect($e->getMessage())->toBePHPEqual('hello world');
    }
  }
}
