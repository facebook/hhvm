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

abstract class E {
  public function foo(): void {}
}

final class A<T as E> {

  public function __construct((function(T): void) $x) {
  }

}

function test1<T as E>(): A<T> {
  return new A(meth_caller('E', 'foo'));
}

function test2<T as E>(): A<T> {
  return new A(meth_caller(E::class, 'foo'));
}
