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

class Foo<T> {
  public function __construct(T $x) {}
}

class Bar {
  // Since Foo's type variable isn't known to be covariant, this is an error
  public function getFooOfBar(): Foo<this> {
    return new Foo($this);
  }
}
