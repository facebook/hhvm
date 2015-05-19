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

class A {}

class :x:dumb {}

function foo(mixed $x): void {
  // Anything goes in XHP
  $x = <x:dumb>{0} {'hello'} {$x} {new A()}</x:dumb>;
}
