<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

function f1(<<Foo(1,2,3), Bar>> $x): void {}

function f2(<<SingleAttribute>> $x): void {}

function f3(<<SingleAttributeWithOneParam(1)>> $x): void {}

function f4(<<SingleAttributeWithTwoParams(1,2)>> $x): void {}

function f5(<<Multiple(1), Attributes(2,3), Foo>> $x): void {}

function f6(<<Data>> ?string $x = null): void {}

function f7(<<Data>> $x = null): void {}

class Blah {
  public function f5(<<Multiple(1), Attributes(2,"b$lah"), Foo>> int $x): void {
    $x = "blah";
    $y = "$x";
  }
}

