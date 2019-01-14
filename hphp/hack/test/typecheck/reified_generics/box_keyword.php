<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Erased<T> {}
class Reified<reify Tr> {}

function Erased_keywordCheck(): void {
  new Erased<Erased>();
  new Erased<reify Erased>(); // relaxed restriction for migration

  new Erased<Reified<int>>();
  new Erased<reify Reified<int>>(); // relaxed restriction for migration
}

function Reified_keywordCheck(): void {
  new Reified<Erased>(); // bad
  new Reified<reify Erased>();

  new Reified<Reified<int>>(); // bad
  new Reified<reify Reified<int>>();
}
