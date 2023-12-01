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


class Test {
  public function bloo(): void {
    $b = 5;
    $map = dict[];
    {
      // UNSAFE
      $map['a'] = $b;
    }
  }
}
