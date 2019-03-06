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
  $x = array('k' => 0, 'k2' => my_php_function());
  $y = array(0, my_php_function());
}

function my_php_function() {}
