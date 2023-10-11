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

class Base {
  public function setA(): this { return $this; }
}

class Child1 extends Base {}
class Child2 extends Base {}
class Child3 extends Base {}
class Child4 extends Base {}

function foo(int $x): Base {
  switch ($x) {
    case 0:
      $thing = new Child1();
      break;
    case 1:
      $thing = new Child2();
      break;
    case 2:
      $thing = new Child3();
      break;
    default:
      $thing = new Child4();
      break;
  }
  return $thing->setA()->setA()->setA()->setA();
}
