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

class MyClass {
  public ?int $field = 0;
  public function doStuff() {}
}

function test<T as ?MyClass>(T $arg): void {
  if ($arg !== null) {
    $arg->doStuff();
    $arg->field;
  }
}

function test2<T as ?string>(T $arg): void {
  if ($arg) {
    $arg[0];
  }
}
