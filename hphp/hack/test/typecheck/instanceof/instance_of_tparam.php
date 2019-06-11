<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function f(mixed $x): KeyedTraversable<string, string> {
  if ($x is KeyedTraversable<_, _>) {
    return $x;
  }

  invariant_violation('lolol');
}

class C<T> {}
class D<T> extends C<T> {}

function g<Tx, Ty>(C<Tx> $x): D<Ty> {
  if ($x is D) {
    return $x;
  }

  invariant_violation('lolol');
}
