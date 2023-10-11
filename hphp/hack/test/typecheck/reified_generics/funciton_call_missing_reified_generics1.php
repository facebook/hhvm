<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Bar<T> {}

function foo<reify T>(): void {
  foo<int>();
  foo<vec>(); // bad
  foo<Bar>(); // bad
}
