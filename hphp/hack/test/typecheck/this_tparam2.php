<?hh
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

class OtherPreparable extends Preparable {}

async function foo(MyPreparable $x): Awaitable<OtherPreparable> {
  return await $x->run();
}
