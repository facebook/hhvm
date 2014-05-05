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

function f(mixed $x): Indexish<string, string> {
  if ($x instanceof Indexish) {
    return $x;
  }

  invariant_violation('lolol');
}

class C<T> {}
class D<T> extends C<T> {}

function g<Tx, Ty>(C<Tx> $x): D<Ty> {
  if ($x instanceof D) {
    return $x;
  }

  invariant_violation('lolol');
}
