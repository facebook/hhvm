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

function any() {
}

function arr<T>(array<T> $x): array<array<T>> {
  return array($x);
}

function f(): void {
  $a = arr(any());
  foreach ($a[0] as $x) {
    $x[] = any();
  }
}
