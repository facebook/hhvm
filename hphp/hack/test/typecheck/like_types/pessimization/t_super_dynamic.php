<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<T, reify Tr> {
  public function f(T $dyn): void {}
  public function g(Tr $dyn): void {}

  public function h(dynamic $d): void {
    $this->f($d);
    $this->g($d);
  }
}
