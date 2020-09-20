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

abstract class MyVector<T> {
  abstract public function map<Tu>((function(T): Tu) $callback): MyVector<Tu>;
  abstract public function at(int $k): T;
  abstract public function isEmpty(): bool;

  static public function newVector(T $x): MyVector<T> {
    throw new Exception();
  }
}

class A {
  public function getName(): string { return 'hello'; }
}


/**
 * There was a weird bug in Typing_generic.rename, that was reproduced with
 * this case. It had to do with variable renaming, and this test doesn't
 * exercise much beyond that
 */
class B {
  public function foo(MyVector<A> $a_vector): void {
    $bars = $a_vector->map(function (A $x) { return MyVector::newVector($x); });
    if (!$bars->isEmpty()) {
      $bars->at(1)->at(1)->getName();
    }
  }
}
