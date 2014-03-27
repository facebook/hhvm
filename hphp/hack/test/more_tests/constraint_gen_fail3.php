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

interface A {
  public function render(): string;
}

class L<T as A> {
  private ?T $data = null;
  
  public function set(T $x): void { $this->data = $x; }
  public function get(): ?T { return $this->data; }
}

class U<T> extends L<A> {
  public function get(): ?T { return parent::get(); }
}



