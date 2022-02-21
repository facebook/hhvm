<?hh // strict
// Copyright 2022-present Facebook. All Rights Reserved.

function foo(string $s) : int {
  switch ($s) {
    default:
      return 0;
    default:
      return 1;
    default:
      return 2;
  }
}
