<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function get_int(): int {
  return 0;
}

class A {
  public function f((int, int)$mt = tuple(get_int(), get_int())): void{}
}

