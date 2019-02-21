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

function meh_while(int $i): void {
  $x = $i;
  do {
  	$i += $x;
  } while($x = true);
}
