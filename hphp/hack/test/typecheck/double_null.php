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

class Env {}

class MyBox<T> {
  private T $data;

  public function __construct(T $x) {
    $this->data = $x;
  }

  public function get(): ?T {
    return $this->data;
  }
}

function nnull<T>(?T $x): T {
  if($x === null) {
    throw new Exception('should not happen');
  }
  return $x;
}

class X {
  private ?Env $x;

  public function test(): Env {
    $blah = new MyBox($this->x);
    $maybe_env = nnull($blah->get());
    return $maybe_env;
  }
}
