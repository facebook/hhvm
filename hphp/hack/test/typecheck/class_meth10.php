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

class C {
  static public function f(int $i): bool {
    $m = class_meth(__CLASS__, 'f');
    hh_show($m);
    return $m($i);
  }
}
