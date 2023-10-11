<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function f1(): array<string, int> {
  return darray[
    'a' => 1,
    'b' => 2,
  ];
}

function f2(): array<string, int> {
  return darray[
    'a' => 1,
    'a' => 2,
  ];
}

function f3(): Map<string, int> {
  return Map {
    'p' => 1,
    'q' => 2,
    'r' => 3,
  };
}

function f4(): Map<string, int> {
  return Map {
    'p' => 1,
    'p' => 2,
    'p' => 3,
  };
}

function f5(): ImmMap<string, int> {
  return ImmMap {
    'x' => 1,
    'y' => 2,
    'z' => 3,
  };
}

function f6(): ImmMap<string, int> {
  return ImmMap {
    'x' => 1,
    'x' => 2,
    'x' => 3,
  };
}
