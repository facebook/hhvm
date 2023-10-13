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

abstract class E {
  public function foo(): void {}
}

final class A<T as E> {

  public function __construct((function(T): void) $x) {}

}

function test1<T as E>(): A<T> {
  return new A(meth_caller('E', 'foo'));
}

function test2<T as E>(): A<T> {
  // Type of $f should be
  //   function(E):void
  $f = meth_caller(E::class, 'foo');
  // Now type of $r should be
  //   A<x>
  // with constraint that
  //   x <: E
  //   function(E):void <: function(x):void
  // i.e.
  //   x <: E (contravariance), so we're ok
  $r = new A($f);
  // Now we need to check the return type
  //   A<x> <: A<T>
  // So by invariance we have x=T, and we're ok
  // because of the T as E constraint
  return $r;
}
