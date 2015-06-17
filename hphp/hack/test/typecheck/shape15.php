<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

type myshape = shape('my_optional_field' => ?int);

function test(): myshape {
  $x = shape('my_optional_field' => 0);
  if (true) {
    $x['my_optional_field'] = null;
  }
  return $x;
}
