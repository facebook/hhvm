<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f<<<__Enforceable>> reify T>(): void {
  3 as (_, _);
  3 as (T, _);
  3 as (_, T);
  3 as (T, T);
}
