<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f<<<__Enforceable>> reify T>(): void {
  3 as shape(
    "a" => _,
    "b" => _
  );
  3 as shape(
    "a" => T,
    "b" => _
  );
  3 as shape(
    "a" => _,
    "b" => T
  );
  3 as shape(
    "a" => T,
    "b" => T
  );
}
