<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class C {
  public static function f(): void {}
}

function f<
  <<__Newable, __Enforceable>> reify Thard as C,
  <<__Soft, __Newable, __Enforceable>> reify Tsoft as C,
>(): void {
  4 as Thard;
  5 as Tsoft;

  new Thard();
  new Tsoft();

  Thard::f();
  Tsoft::f();
}
