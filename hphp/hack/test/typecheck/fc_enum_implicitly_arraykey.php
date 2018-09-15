<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

enum FooInt: int {
  FOO_VALUE_ONE = 1;
  FOO_VALUE_TWO = 2;
}

enum FooStr: string {
  FOO_VALUE_BAR = 'foobar';
  FOO_VALUE_BAZ = 'foobaz';
}

enum BarInt: int as int {
  BAR_VALUE_ONE = 1;
  BAR_VALUE_TWO = 2;
}

enum BarStr: string as string {
  BAR_VALUE_BAR = 'barbar';
  BAR_VALUE_BAZ = 'barbaz';
}

function takes_arraykey(arraykey $arg): void {
  // do nada
}

function main(): void {
  // ensure opaque enum's values pass arraykey typehint
  takes_arraykey(FooInt::FOO_VALUE_ONE);
  takes_arraykey(FooStr::FOO_VALUE_BAR);

  // ensure constrained enum's values pass arraykey typehint
  takes_arraykey(BarInt::BAR_VALUE_ONE);
  takes_arraykey(BarStr::BAR_VALUE_BAR);
}
