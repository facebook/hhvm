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

function f1() {
  for ($i=0, $j=0; $i<10; $i++, $j++) {
    echo $i, "\t", $j, "\n";
  }
}

function f2() {
  for ($i=0;$i<10;) {
  }
}

function f3() {
  $i = 0;
  for (; $i<0;){
  }
}
