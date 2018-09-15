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

type my_shape = shape(
  'x' => int,
  'y' => bool,
);

function foo(bool $cond): my_shape {
  $s = shape('x' => 0);
  if ($cond) {
    $s['y'] = true;
  }
  return $s;
}
