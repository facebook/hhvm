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


function main(): void {
  $x = new Vector(null);
  $z = Vector {1, 2, 3};
  $t = Map {1 => 1, 2 => 2, 3 => 3};
  $y = new Map(null);
  $y['ff'] = 0;

  $i = 0;
  foreach($x as $k => $v) {
    $i += $k + $v;
  }

  $j = 0;
  foreach($x as $v) {
    $j += $v;
  }

  $f = 0;
  foreach($y as $k => $v) {
    $f += $v;
    if ($k === 'ff') { $f++; }
  }

  if ($i === 9 && $j == 6) {
    echo 'OK';
  }
  else {
    echo 'FAILURE: test4.1';
  }
}
