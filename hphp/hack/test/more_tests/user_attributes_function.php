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

class blah {
  <<Foo(1,2,3), Bar>>
  public function f1(): void {}
}

<<SingleAttribute>> function f2(): void {}

<<SingleAttributeWithOneParam(1)>> function f3(): void {}

<<SingleAttributeWithTwoParams(1,2)>> function f4(): void {}

<<Multiple(1), Attributes(2,3), Foo>> function f5(): void {}
