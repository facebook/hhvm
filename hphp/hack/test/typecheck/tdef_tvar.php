<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

type TDef<T> = T;

class A<T> {
  public function __construct(TDef<T> $x) {
  }
}

function test(): void {
  $x = new A(0);
}

