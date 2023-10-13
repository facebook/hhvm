<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

type myshape = shape(
  'field1' => int,
  'field2' => bool,
);
type smaller_shape = shape(
  'field1' => int,
  'field2' => bool,
);

function cmp<T>(T $x, T $y): void {}

function test(myshape $x, smaller_shape $y): void {
  cmp($x, $y);
}
