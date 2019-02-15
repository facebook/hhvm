<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Erased<T> {}
class Reified<reify T> {}

function new_keywordCheck(): void {
  new Erased();
  new Erased<int>();

  new Reified(); // bad
  new Reified<int>();
}
