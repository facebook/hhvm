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

class Blah {}

class :x:xx extends Blah {
  attribute Map<string, int> k;
}

class :x:xy {}

class A {}

function test(): void {
  $x = <x:xx k={Map {}}></x:xx>;
}
