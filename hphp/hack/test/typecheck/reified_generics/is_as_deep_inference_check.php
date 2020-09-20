<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Reified<reify T> {}

function f(Reified<Reified<
  Reified<int>
>> $r) {}

function g(): void {
  $a = 3 as Reified<Reified<
    int
  >>;

  f($a);
}
