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

function any() {
}

function arr<T>(array<T> $x): array<array<T>> {
  return varray[$x];
}

function f(): void {
  $a = arr(any());
  foreach ($a[0] as $x) {
    $x[] = any();
  }
}
