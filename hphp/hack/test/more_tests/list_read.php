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

class Foo {
  public static Map<int, (int, int)> $data = Map {};

  private int $a, $b;
  public function __construct(int $idx) {
    list($this->a, $this->b) = self::$data[$idx];
  }
}
