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

function test(): void {
  $x = array('k' => 0, 'k2' => my_php_function());
  $x['k2'][0] = 0;
  $y = array(0, my_php_function());
  $y[0][0] = 0;
}
