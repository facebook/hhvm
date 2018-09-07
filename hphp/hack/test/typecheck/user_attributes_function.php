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

class Foo implements HH\FunctionAttribute, HH\MethodAttribute { public function __construct(int... $x) {} }
class Bar implements HH\MethodAttribute {}
class SingleAttribute implements HH\FunctionAttribute {}
class SingleAttributeWithOneParam implements HH\FunctionAttribute { public function __construct(public int $x) {} }
class SingleAttributeWithTwoParams implements HH\FunctionAttribute { public function __construct(public int $x, public int $y) {} }
class Multiple implements HH\FunctionAttribute { public function __construct(public int $x) {} }
class Attributes implements HH\FunctionAttribute { public function __construct(public int $x, public int $y) {} }

class blah {
  <<Foo(1,2,3), Bar>>
  public function f1(): void {}
}

<<SingleAttribute>> function f2(): void {}

<<SingleAttributeWithOneParam(1)>> function f3(): void {}

<<SingleAttributeWithTwoParams(1,2)>> function f4(): void {}

<<Multiple(1), Attributes(2,3), Foo>> function f5(): void {}
