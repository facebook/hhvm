<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

<<__ConsistentConstruct>>
class X {}

class Y {}

class C<Tx as X, Ty as Y> {
  // correctly resolves X constraint via Tx
  public function good<
    <<__Newable>> reify Tgood as Tx
  >(): void {}

  public function bad<
    <<__Newable>> reify Tbad as Ty
  >(): void {}
}
