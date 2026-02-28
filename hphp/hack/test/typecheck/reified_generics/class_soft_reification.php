<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Erased<T> {}
class SoftReified<<<__Soft>> reify T> {}
class Reified<reify T> {}

function new_keywordCheck(): void {
  new Erased();
  new Erased<int>();

  new SoftReified(); // bad
  new SoftReified<int>();

  new Reified(); // bad
  new Reified<int>();
}

function reification_test<
  Terase,
  <<__Soft>> reify Tsoft,
  reify Thard
>(): void {
  new Erased<Terase>();
  new Erased<Tsoft>();
  new Erased<Thard>();

  new SoftReified<Terase>();
  new SoftReified<Tsoft>();
  new SoftReified<Thard>();

  new Reified<Terase>();
  new Reified<Tsoft>();
  new Reified<Thard>();
}
