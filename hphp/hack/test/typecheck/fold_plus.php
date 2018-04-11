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

function test(): void {
  $x = Vector {};
  $x[] = 0;
  $x[] = 0.0;
  $y = $x[0] - 1;
}
