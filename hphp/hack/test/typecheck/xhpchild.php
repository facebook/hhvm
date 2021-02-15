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

class :x extends XHPTest implements XHPChild {}

function f1(XHPChild $x): void {}

function f2(): void {
  f1("hello");
  f1(varray["hello", "world"]);
  f1(<x>hi</x>);
}
