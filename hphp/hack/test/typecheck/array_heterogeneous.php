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

function test(): void {
  $x = darray['k' => 0, 'k2' => my_php_function()];
  $x['k2'][0] = 0;
  $y = varray[0, my_php_function()];

  $index = 0;
  $y[$index] = 0; // force conversion from tuple-like to vec-like

  $y[0][0] = 0;
}

function my_php_function() {}
