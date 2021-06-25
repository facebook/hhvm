<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Reified<reify T> {}
interface I {}

function f(Reified<Reified<
  Reified<int>
>> $r) : void {}

function g(I $i): void {
  $a = $i as Reified<Reified<
    int
  >>;

  f($a);
}
