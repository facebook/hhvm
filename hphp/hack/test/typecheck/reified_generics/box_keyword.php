<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Erased<T> {}
class Reified<reify Tr> {}

function Erased_keywordCheck(): void {
  new Erased<Erased>(); // bad, Erased requires argument
  new Erased<Erased<_>>();

  new Erased<Reified<int>>();
  new Erased<Reified<int>>();
}

function Reified_keywordCheck(): void {
  new Reified<Erased>(); // bad, Erased requires argument
  new Reified<Erased<_>>();

  new Reified<Reified<int>>();
  new Reified<Reified<int>>();
}
