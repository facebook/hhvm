<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Erased<T> {}
class SoftReified<<<__Soft>> reify T> {}
class Reified<reify T> {}

function new_keywordCheck(): void {
  new Erased();
  new Erased<int>();
  new Erased<reify int>();

  new SoftReified(); // for migration
  new SoftReified<int>(); // for migration
  new SoftReified<reify int>();

  new Reified(); // bad
  new Reified<int>(); // bad
  new Reified<reify int>();
}
