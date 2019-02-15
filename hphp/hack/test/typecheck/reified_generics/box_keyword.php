<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Erased<T> {}
class Reified<reify Tr> {}

function Erased_keywordCheck(): void {
  new Erased<Erased>();
  new Erased<Erased>();

  new Erased<Reified<int>>();
  new Erased<Reified<int>>();
}

function Reified_keywordCheck(): void {
  new Reified<Erased>();
  new Reified<Erased>();

  new Reified<Reified<int>>();
  new Reified<Reified<int>>();
}
