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
  new Reified<Erased>(); // bad : Erased requires argument
  new Reified<Erased<int>>();
  new Reified<Erased<int, string>>(); // bad

  new Reified<Reified>(); // bad, inner Reified requires argument
  new Reified<Reified<int>>();
  new Reified<Reified<int, string>>(); // bad
}
