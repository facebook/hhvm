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

class Foo {
  private Map<string, string> $prop = Map {};
  public function bar() {
    if ($this->prop) {
    }
    $this->prop = null;
  }
}
