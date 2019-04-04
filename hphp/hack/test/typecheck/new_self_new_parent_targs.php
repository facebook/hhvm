<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<Tc> {}

class D<Td, Tdc> extends C<Tdc> {
  public function new_self(): void {
    $s = new self();
    hh_show($s);
  }

  public function new_parent(): void {
    $p = new parent();
    hh_show($p);
  }
}
