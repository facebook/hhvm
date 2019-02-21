<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

interface IFoo<T> {}

class Bar {}

final class Foo<T as Bar> implements IFoo<T> {

  public function me(): this {
    return $this;
  }
}
