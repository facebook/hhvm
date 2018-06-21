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

class A<T as C> extends C {
  public function bar(T $x): T {
    return $x;
  }

}

class B<T as A<C>> extends A<T> {}

class D extends B<D> {}
