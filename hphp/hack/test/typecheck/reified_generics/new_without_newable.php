<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__ConsistentConstruct>>
class X {}

class C<
  <<__Newable>> reify Tc as X,
> {
  public function not_newable<
    reify Tnn as Tc
  >(): void {
    new Tnn();
  }

  public function newable<
    <<__Newable>> reify Tnn as Tc
  >(): void {
    new Tnn();
  }

  public function cls_newable(): void {
    new Tc();
  }
}
