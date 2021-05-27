<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Reified<reify T> {}
interface I {}
class D implements I {}

function f(Reified<Reified<
  Reified<int>
>> $r) : void {}

function g(): void {
  $a = new D() as Reified<Reified<
    int
  >>;

  f($a);
}
