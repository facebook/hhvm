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

type myshape = shape('my_optional_field' => ?int);

function test(): myshape {
  $x = shape('my_optional_field' => 0);
  if (true) {
    $x['my_optional_field'] = null;
  }
  return $x;
}
