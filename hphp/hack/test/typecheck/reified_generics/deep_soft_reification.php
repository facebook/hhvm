<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Erased<T> {}
class Reified<reify T> {}

function new_targCheck(): void {
  new Erased<Erased>();
  new Erased<Erased<int>>();

  new Erased<Reified>(); // bad
  new Erased<Reified<int>>();
}
