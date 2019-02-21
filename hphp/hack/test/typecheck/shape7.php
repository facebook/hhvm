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

type myshape = shape(
  'field1' => int,
  'field2' => bool,
  ...
);

function test(): myshape {
  $x = shape('field1' => 0);
  $x['undef'] = 0;
  return $x;
}
