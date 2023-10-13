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

interface IFoo<T> {
  public function run(): Awaitable<T>;
}

class Foo implements IFoo<Awaitable<this>> {
  public function run(): Awaitable<Awaitable<this>> {
    throw new Exception();
  }
}
