<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Erased<T> {}
class Reified<reify Tr> {}

function Erased_arityCheck(): void {
  new Erased<int, string>(); // already an error for this

  new Erased<Erased<int>>();
  new Erased<Erased<int, string>>(); // bad

  new Erased<Reified<int>>();
  new Erased<Reified<int, string>>(); // bad
}

function Reified_arityCheck(): void {
  new Reified<reify Erased>();
  new Reified<reify Erased<int>>();
  new Reified<reify Erased<int, string>>(); // bad

  new Reified<reify Reified>(); // bad, Reified requires parameters
  new Reified<reify Reified<int>>();
  new Reified<reify Reified<int, string>>(); // bad
}
