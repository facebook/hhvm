<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function erased<T>(): void {}
function softReified<<<__Soft>> reify T>(): void {}
function reified<reify T>(): void {}

function call_keywordCheck(): void {
  erased();
  erased<int>();
  erased<reify int>();

  softReified(); // for migration
  softReified<int>(); // for migration
  softReified<reify int>();

  reified(); // bad
  reified<int>(); // bad
  reified<reify int>();
}
