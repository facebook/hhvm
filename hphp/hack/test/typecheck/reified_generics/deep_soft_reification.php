<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Erased<T> {}
class SoftReified<<<__Soft>> reify T> {}
class Reified<reify T> {}

function new_targCheck(): void {
  new Erased<Erased<_>>();
  new Erased<Erased<int>>();

  new Erased<SoftReified>(); // bad
  new Erased<SoftReified<int>>();

  new Erased<Reified>(); // bad
  new Erased<Reified<int>>();
}

function reification_test<
  Terase,
  <<__Soft>> reify Tsoft,
  reify Thard
>(): void {
  new Erased<Erased<Terase>>();
  new Erased<Erased<Tsoft>>();
  new Erased<Erased<Thard>>();

  new Erased<SoftReified<Terase>>();
  new Erased<SoftReified<Tsoft>>();
  new Erased<SoftReified<Thard>>();

  new Erased<Reified<Terase>>();
  new Erased<Reified<Tsoft>>();
  new Erased<Reified<Thard>>();
}
