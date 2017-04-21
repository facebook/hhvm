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

class C {
  public function bar(): void {
    echo 'bar';
  }
}
class Wrapper<T> {
  public function __construct(private T $item) {}
  public function foo(): T {
    return $this->item;
  }
}

function use_with_ints((function(Wrapper<int>): C) $caller): void {
  $b = $caller(new Wrapper(3));
  $b->bar();
}

function breakIt(): void {
  $caller = meth_caller('Wrapper', 'foo');
  use_with_ints($caller);
}

/* HH_FIXME[1002] */
breakIt();
