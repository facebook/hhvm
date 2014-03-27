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

class C {}

class A<T as C> extends C {
  public function bar(T $x): T { return $x; }
  
}

class B<T as B<A>> extends A<T> {
}

class D extends B<D> {
}
