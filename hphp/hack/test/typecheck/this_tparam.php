<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

interface IPreparable<T> {
  public function run(): Awaitable<T>;
}

class Preparable implements IPreparable<this> {
  public async function run(): Awaitable<this> {
    return $this;
  }
}

class MyPreparable extends Preparable {}

async function foo1(Preparable $x): Awaitable<Preparable> {
  return await $x->run();
}

async function foo2(MyPreparable $x): Awaitable<MyPreparable> {
  return await $x->run();
}

async function foo3(MyPreparable $x): Awaitable<Preparable> {
  return await $x->run();
}

abstract class GeneratorLegacyPreparable implements IPreparable<this> {

  <<__Memoize>>
  private async function __genGen(): Awaitable<this> {
    await $this->gen();
    return $this;
  }

  public async function run(): Awaitable<this> {
    return await $this->__genGen();
  }

  abstract public function gen(): Awaitable<void>;
}

class MyLegacyPrep extends GeneratorLegacyPreparable {
  public function __construct(private MyPreparable $x) {}
  public async function gen(): Awaitable<void> {
    await $this->x->run();
  }
}
