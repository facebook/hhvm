<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A<reify Ta> {}
class C<reify Tb> extends A<Tb> {
  public function f(): void {
    $c = new self();
    hh_show($c); // C<[unresolved]> TODO:T41884303
    $a = new parent();
    hh_show($a); // A<[unresolved]> TODO:T41884303
  }
}
