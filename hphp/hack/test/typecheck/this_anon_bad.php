<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

<<__ConsistentConstruct>>
class Foo {
  public function getAnon1(): (function(this): int) {
    return function(this $x): int {
      return 4;
    };
  }

  public function test(): void {
    $func = $this->getAnon1();

    // Can pass in $this
    $func($this);

    // Can pass an alias of $this
    $this_alias = $this;
    $func($this_alias);

    // Can pass new static()
    $func(new static());

    // But not new Foo()
    $func(new Foo());
  }
}
