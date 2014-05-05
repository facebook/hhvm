<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class Preparable implements Awaitable<this> {}

class MyPreparable extends Preparable {}

async function foo1(Preparable $x): Awaitable<Preparable> {
  return await $x;
}

async function foo2(MyPreparable $x): Awaitable<MyPreparable> {
  return await $x;
}

async function foo3(MyPreparable $x): Awaitable<Preparable> {
  return await $x;
}

abstract class GeneratorLegacyPreparable implements Awaitable<this> {
  private ?WaitHandle<this> $wh = null;
  private bool $started = false;

  final public function getWaitHandle(): WaitHandle<this> {
    if ($this->wh === null) {
      invariant($this->started === false, 'cannot depend on $this');
      $this->started = true;
      $wh = $this->__genGen();
      invariant($wh instanceof WaitHandle, 'trust me');
      $this->wh = $wh;
    }
    return $this->wh;
  }

  private async function __genGen(): Awaitable<this> {
    await $this->gen();
    return $this;
  }

  abstract public function gen(): Awaitable<void>;
}

class MyLegacyPrep extends GeneratorLegacyPreparable {
  public function __construct(private MyPreparable $x) {}
  public async function gen(): Awaitable<void> {
    await $this->x;
  }
}
