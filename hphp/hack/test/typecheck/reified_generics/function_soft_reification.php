<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function erased<T>(): void {}
function reified<reify T>(): void {}

function call_keywordCheck(): void {
  erased();
  erased<int>();

  reified(); // bad
  reified<int>();
}
