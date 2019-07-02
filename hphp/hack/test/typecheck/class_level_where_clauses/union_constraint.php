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

// We want to demonstrate that tparam_constr works with where_constr
abstract class A {}
interface B {}
abstract class C extends A implements B {}  // C subtype A and B
abstract class UnionConstr <T super A> where T as C {  // No error thrown here
  abstract public function get(): T;
  public function run(): C {
    return $this->get();
  }
}
