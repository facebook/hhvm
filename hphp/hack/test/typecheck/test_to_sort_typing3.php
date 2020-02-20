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

class A {

  public function myfun($a) {
    $a[] = 'my_value';
    return $a;
  }

  public function myfun2($a) {
    return $a['a'];
  }

  public function main($a) {
    $x = $this->myfun($a);
    $t = darray['e' => 1];
    $z = $t['f'];
  }
}
