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

interface SomethingElse<T> {}

class Foo implements SomethingElse<this> {}

function expects1<T as Foo>(SomethingElse<T> $foo): void {}

function expects2(SomethingElse<Foo> $foo): void {}

function test(Foo $foo): void {
  // No error since Foo is an 'as' constraint on a generic
  expects1($foo);

  // Error because SomethingElse<expression_dependent<Foo>> is different than
  // SomethingElse<Foo>
  expects2($foo);
}
