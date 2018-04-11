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

class C {}

function f(arraykey $k1, arraykey $k2): arraykey {
  return $k2;
}

function test(): void {
  $c = new C();
  f($c, 2);
}
