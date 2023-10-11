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

// My templated class
abstract class TestObj<T> {}
class TestBool extends TestObj<bool> {}

// I'm trying to make this work, but get:
abstract class VisitorObj<T, Tobj as TestObj<T> > {
  abstract public function getObj(): Tobj;
}
class VisitorBool extends VisitorObj<bool, TestBool> {
  public function getObj(): TestBool {
    return new TestBool();
  }
}
