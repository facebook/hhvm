<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function erased<T>(): void {}
function softReified<<<__Soft>> reify T>(): void {}
function reified<reify T>(): void {}

function call_keywordCheck(): void {
  erased();
  erased<int>();

  softReified(); // bad
  softReified<int>();

  reified(); // bad
  reified<int>();
}

function reification_test<
  Terase,
  <<__Soft>> reify Tsoft,
  reify Thard
>(): void {
  erased<Terase>();
  erased<Tsoft>();
  erased<Thard>();

  softReified<Terase>();
  softReified<Tsoft>();
  softReified<Thard>();

  reified<Terase>();
  reified<Tsoft>();
  reified<Thard>();
}
