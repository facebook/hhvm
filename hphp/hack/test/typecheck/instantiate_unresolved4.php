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

/* HH_FIXME[4336] */
function foo<T>(mixed $x): array<T> {
}

function a<T>(): array<T> {
  return varray[];
}

function f(): void {
  $a = a();
  $b = foo($a);
}
