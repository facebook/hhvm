<?hh
/**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

type A<T as string> = vec<T>;

function f0<T as string>(A<T> $foo): T {
  return $foo[0];
}

function f1(vec<string> $x): void {
  f0($x);
}

function f2(vec<int> $x): void {
  f0($x);
}
